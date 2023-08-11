#include "GameplayAbilitySystem/GA_LookAt.h"

#include "GameplayAbilitySystem/MyAttributes.h"
#include "MyGameplayTags.h"
#include "GameplayAbilitySystem/GE_TorqueCCW.h"
#include "GameplayAbilitySystem/GE_TorqueCW.h"
#include "GameplayAbilitySystem/MyAbilitySystemComponent.h"
#include "Logging/StructuredLog.h"
#include "Modes/MyGameInstance.h"
#include "UE5Coro/LatentAwaiters.h"

using namespace UE5Coro;
using namespace UE5Coro::GAS;

UGA_LookAt::UGA_LookAt()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    
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
    {
        co_await Latent::Cancel();
    }
    auto* ASC = UMyAbilitySystemComponent::Get(ActorInfo);
    const auto& Tag = FMyGameplayTags::Get();
    const float OmegaMax = ASC->GetNumericAttribute(UAttrSetAcceleration::GetOmegaMaxAttribute());
    const float AlphaMax = ASC->GetNumericAttribute(UAttrSetAcceleration::GetTorqueMaxAttribute());
    
    // time to transition from idle pose to torque pose
    constexpr float TransitionTime = 0.2f;

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
    double TIdle = FMath::Max(0., DeltaThetaAbs / OmegaBar - OmegaBar / AlphaMax - 2. * TransitionTime);
    
    const FGameplayTag CuePose1 = DeltaTheta > 0 ? Tag.CuePoseTurnCCW : Tag.CuePoseTurnCW;
    const FGameplayTag CuePose2 = DeltaTheta > 0 ? Tag.CuePoseTurnCW : Tag.CuePoseTurnCCW;

    UE_LOGFMT
        ( LogMyGame
        , Display
        , "Ability LookAt Begin: DeltaTheta: {DELTATHETA}, TTorque1: {TTORQUE1}, TIdle: {TIdle}, TTorque2: {TTORQUE2}, OmegaBar: {OMEGABAR}, Omega: {OMEGA}"
        , DeltaTheta
        , TTorque1
        , TIdle
        , TTorque2
        , OmegaBar
        , MyPawn->GetOmega()
        );

    // TODO: only remove, when needed; and don't remove where it gets activated again
    RemoveActiveGameplayEffect(TorqueHandle, *ActorInfo, ActivationInfo);

    ASC->AddGameplayCueUnlessExists(Tag.CueAccelerateShowThrusters);

    if(TTorque1 > 0)
    {
        // TODO: allow direct switch from Pose2 to Pose1

        const bool b1 = ASC->RemoveGameplayCueIfExists(CuePose2);
        const bool b2 = ASC->AddGameplayCueUnlessExists(CuePose1);
        if(b1 || b2)
            co_await Latent::UntilDelegate(ASC->OnAnimStateFullyBlended);

        const auto SpecTorque1 =
            MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, DeltaTheta > 0 ? GE_TorqueCCW : GE_TorqueCW);
        TorqueHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecTorque1);
        co_await Latent::Seconds(TTorque1);
        RemoveActiveGameplayEffect(TorqueHandle, *ActorInfo, ActivationInfo);

        const float Omega = MyPawn->GetOmega(); 
        if(FMath::Abs(Omega - OmegaBar) > 0.01)
            UE_LOGFMT(LogMyGame, Warning, "target rotation speed not reached: Omega = {OMEGA} ({OMEGABAR})"
                , Omega, OmegaBar);
    }

    if(TTorque1 >= 0.)
    {
        ASC->RemoveGameplayCueIfExists(CuePose1);
        //co_await Latent::UntilDelegate(ASC->OnAnimStateFullyBlended);
        co_await Latent::Seconds(TransitionTime);
        
        co_await Latent::Seconds(TIdle);
        
        if(ASC->AddGameplayCueUnlessExists(CuePose2))
            co_await Latent::UntilDelegate(ASC->OnAnimStateFullyBlended);
    }
    
    const auto SpecTorque2 =
        MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, DeltaTheta > 0 ? GE_TorqueCW : GE_TorqueCCW);
    TorqueHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecTorque2);
    co_await Latent::Seconds(TTorque2);
    RemoveActiveGameplayEffect(TorqueHandle, *ActorInfo, ActivationInfo);

    const bool b3 = ASC->RemoveGameplayCueIfExists(CuePose2);
    const bool b4 = ASC->RemoveGameplayCueIfExists(CuePose1);
    if(b3 || b4)
        co_await Latent::UntilDelegate(ASC->OnAnimStateFullyBlended);
    
    ASC->RemoveGameplayCue(Tag.CueAccelerateShowThrusters);

    if(const auto OmegaLog = Cast<AMyPawn>(ActorInfo->OwnerActor)->GetOmega(); FMath::Abs(OmegaLog) <= 0.001)
        UE_LOGFMT(LogMyGame, Display, "Ability LookAt finished");
    else
        UE_LOGFMT(LogMyGame, Error, "Ability LookAt finished, but Omega > 0.001: Omega = {OMEGA}", OmegaLog);
    co_return;
}
