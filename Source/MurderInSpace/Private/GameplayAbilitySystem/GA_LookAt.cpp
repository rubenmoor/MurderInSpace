#include "GameplayAbilitySystem/GA_LookAt.h"

#include "GameplayAbilitySystem/MyAttributes.h"
#include "MyGameplayTags.h"
#include "GameplayAbilitySystem/GE_TorqueCCW.h"
#include "GameplayAbilitySystem/GE_TorqueCW.h"
#include "GameplayAbilitySystem/MyAbilitySystemComponent.h"
#include "UE5Coro/LatentAwaiters.h"

using namespace UE5Coro;
using namespace UE5Coro::GAS;

UGA_LookAt::UGA_LookAt()
{
    const auto& Tag = FMyGameplayTags::Get();

    AbilityTags.AddTag(Tag.AbilityLookAt);
    
    FAbilityTriggerData TriggerData;
    TriggerData.TriggerTag = Tag.AbilityLookAt;
    AbilityTriggers.Add(TriggerData);

    GE_TorqueCCW = UGE_TorqueCCW::StaticClass();
    GE_TorqueCW = UGE_TorqueCW::StaticClass();
}

FAbilityCoroutine UGA_LookAt::ExecuteAbility(FGameplayAbilitySpecHandle Handle,
                                             const FGameplayAbilityActorInfo* ActorInfo, FGameplayAbilityActivationInfo ActivationInfo,
                                             const FGameplayEventData* TriggerEventData)
{
    if(!CommitAbility(Handle, ActorInfo, ActivationInfo))
        co_await Latent::Cancel();
    
    if(auto* OnBlockingAbilityEnded = TurnBlocked(Handle, ActorInfo))
        co_await Latent::UntilDelegate(*OnBlockingAbilityEnded);
    
    auto* ASC = UMyAbilitySystemComponent::Get(ActorInfo);
    ASC->AbilityAwaitingTurn = FGameplayAbilitySpecHandle();
    
    checkf(TriggerEventData, TEXT("Use UAbilitySystemComponent::SendGameplayEvent to activate this ability"))
    
    const float OmegaMax = ASC->GetNumericAttribute(UAttrSetAcceleration::GetOmegaMaxAttribute());
    const float AlphaMax = ASC->GetNumericAttribute(UAttrSetAcceleration::GetTorqueMaxAttribute());
    
    const auto* MyPawn = Cast<AMyPawn>(ActorInfo->OwnerActor);
    // this omega is obsolete after any co_await
    const float OmegaBegin = MyPawn->GetOmega();
    
    const float AngleLookAt = TriggerEventData->EventMagnitude;
    const float DeltaThetaInEfficient =
        ( FQuat(FVector::UnitZ(), AngleLookAt) * MyPawn->GetActorQuat().Inverse()
        ).GetTwistAngle(FVector::UnitZ());

    // get a time-efficient delta theta that allows rotation by |DeltaTheta| > PI,
    // if current rotation Omega has opposite sign to DeltaTheta
    const float DeltaTheta =
          FMath::Sign(OmegaBegin) * DeltaThetaInEfficient
        < pow(OmegaBegin, 2) / AlphaMax + TransitionTime * FMath::Abs(OmegaBegin) - PI
            ? DeltaThetaInEfficient + FMath::Sign(OmegaBegin) * TWO_PI
            : DeltaThetaInEfficient;
    const float DeltaThetaAbs = FMath::Abs(DeltaTheta);

    // calculate the maximum radial velocity: for small values of DeltaTheta, it's smaller than OmegaMax
    const float Term = AlphaMax * TransitionTime;
    const float OmegaBar = FMath::Min(OmegaMax, FMath::Sqrt(FMath::Pow(Term, 2) + AlphaMax * DeltaThetaAbs) - Term);
    check(OmegaBar > 0)

    // clamp omega to omega max
    // otherwise sometimes omega overshoots omega max,
    // resulting in TTorque1 being slightly negative and the pawn hitting the breaks
    // clamped, TTorque1 will be exactly 0
    const float OmegaBeginClamped = FMath::Sign(OmegaBegin) * FMath::Min(OmegaMax, FMath::Abs(OmegaBegin));
    const float TTorque1 = (OmegaBar - FMath::Sign(DeltaTheta) * OmegaBeginClamped) / AlphaMax;

    const float TTorque2 = FMath::Sign(DeltaTheta) * OmegaBegin > OmegaBar
        ? FMath::Abs(OmegaBegin) / AlphaMax
        : OmegaBar / AlphaMax;

    // TIdle > 0, the values < 0 are small in absolute terms, so it shouldn't matter
    const double TIdle = FMath::Max(0., DeltaThetaAbs / OmegaBar - OmegaBar / AlphaMax - 2. * TransitionTime);
    
    const auto& Tag = FMyGameplayTags::Get();
    const FGameplayTag CuePose1 = DeltaTheta > 0 ? Tag.CuePoseTorqueCCW : Tag.CuePoseTorqueCW;
    const FGameplayTag CuePose2 = DeltaTheta > 0 ? Tag.CuePoseTorqueCW : Tag.CuePoseTorqueCCW;
    const auto SpecTorque1 =
        MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, DeltaTheta > 0 ? GE_TorqueCCW : GE_TorqueCW);
    const auto SpecTorque2 =
        MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, DeltaTheta > 0 ? GE_TorqueCW : GE_TorqueCCW);

    ASC->AddGameplayCueUnlessExists(Tag.CueShowThrusters);

    // phase 1: get rotation started

    if(TTorque1 > 0)
    {
        const bool b1 = ASC->RemoveGameplayCueIfExists(CuePose2);
        const bool b2 = ASC->AddGameplayCueUnlessExists(CuePose1);
        if(b1 || b2)
            co_await Latent::UntilDelegate(ASC->OnAnimStateFullyBlended);
        
        const auto* GE = ASC->GetActiveGameplayEffect(TorqueHandle);
        if(!GE || GE->Spec.Def != SpecTorque1.Data->Def)
        {
            if(GE)
                RemoveActiveGameplayEffect(TorqueHandle, *ActorInfo, ActivationInfo);
            TorqueHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecTorque1);
        }
        co_await Latent::Seconds(TTorque1);
    }

    // phase 2: keep rotating without torque
    
    if(TTorque1 >= 0.)
    {
        RemoveActiveGameplayEffect(TorqueHandle, *ActorInfo, ActivationInfo);
        
        ASC->RemoveGameplayCueIfExists(CuePose1);
        co_await Latent::Seconds(TransitionTime);
        
        co_await Latent::Seconds(TIdle);
    }
        
    // phase 3: stop rotation

    if(ASC->AddGameplayCueUnlessExists(CuePose2))
        co_await Latent::UntilDelegate(ASC->OnAnimStateFullyBlended);

    const auto* GE = ASC->GetActiveGameplayEffect(TorqueHandle);
    if(!GE || GE->Spec.Def != SpecTorque2.Data->Def)
    {
        if(GE)
            RemoveActiveGameplayEffect(TorqueHandle, *ActorInfo, ActivationInfo);
        TorqueHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecTorque2);
    }
    co_await Latent::Seconds(TTorque2);
    RemoveActiveGameplayEffect(TorqueHandle, *ActorInfo, ActivationInfo);

    const bool b3 = ASC->RemoveGameplayCueIfExists(CuePose2);
    const bool b4 = ASC->RemoveGameplayCueIfExists(CuePose1);
    if(b3 || b4)
        co_await Latent::UntilDelegate(ASC->OnAnimStateFullyBlended);
    
    ASC->RemoveGameplayCueIfExists(Tag.CueShowThrusters);
}
