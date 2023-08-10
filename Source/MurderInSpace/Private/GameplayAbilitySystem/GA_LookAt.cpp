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
        UE_LOGFMT(LogMyGame, Warning, "UGA_LookAt: could not commit ability");
        co_await Latent::Cancel();
    }
    auto* ASC = UMyAbilitySystemComponent::Get(ActorInfo);
    const auto& Tag = FMyGameplayTags::Get();
    const float OmegaMax = ASC->GetNumericAttribute(UAttrSetAcceleration::GetOmegaMaxAttribute());
    const float AlphaMax = ASC->GetNumericAttribute(UAttrSetAcceleration::GetTorqueMaxAttribute());
    // time to transition from idle pose to torque pose
    constexpr float TransitionTime = 0.2f;

    const auto* MyPawn = Cast<AMyPawn>(ActorInfo->OwnerActor);
    const float AngleLookAt = TriggerEventData->EventMagnitude;
    const float DeltaTheta = (FQuat(FVector::UnitZ(), AngleLookAt) * MyPawn->GetActorQuat().Inverse()).GetTwistAngle(FVector::UnitZ());
    const float DeltaThetaAbs = FMath::Abs(DeltaTheta);

    // calculate the maximum radial velocity: for small values of DeltaTheta, it's smaller than OmegaMax
    const float Term = AlphaMax * TransitionTime;
    const float OmegaBar = FMath::Min(OmegaMax, FMath::Sqrt(FMath::Pow(Term, 2) + AlphaMax * DeltaThetaAbs) - Term);
    check(OmegaBar > 0)
    
    const float TTorque1 = (OmegaBar - FMath::Sign(DeltaTheta) * MyPawn->GetOmega()) / AlphaMax;
    const float TTorque2 = OmegaBar / AlphaMax + (TTorque1 > 0 ? 0 : -TTorque1);

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

    if(!ASC->HasMatchingGameplayTag(Tag.CueAccelerateShowThrusters))
        ASC->AddGameplayCue(Tag.CueAccelerateShowThrusters);

    //if(ASC->GetActiveGameplayEffect(TorqueHandle))
    //    verify(RemoveActiveGameplayEffect(TorqueHandle, *ActorInfo, ActivationInfo))

    if(TTorque1 > 0)
    {
        //if(ASC->HasMatchingGameplayTag(CuePose2))
        //{
        //    // go to idle pose first
        //    ASC->RemoveGameplayCue(CuePose2);
        //    
        //    // make up for movement while changing pose
        //    TIdle += TransitionTime;
        //    
        //    co_await Latent::UntilDelegate(ASC->OnStateFullyBlended);
        //}

        // TODO: allow direct switch from Pose2 to Pose1
        co_await ASC->UntilPoseFullyBlended(CuePose2, EPoseCue::Remove);
        co_await ASC->UntilPoseFullyBlended(CuePose1, EPoseCue::Add);

        auto SpecTorque1 = MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, DeltaTheta > 0 ? GE_TorqueCCW : GE_TorqueCW);
        TorqueHandle     = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecTorque1);
        co_await Latent::Seconds(TTorque1);
        verify(RemoveActiveGameplayEffect(TorqueHandle, *ActorInfo, ActivationInfo))
    }

    if(TTorque1 >= 0.)
    {
        co_await ASC->UntilPoseFullyBlended(CuePose1, EPoseCue::Remove);
        co_await Latent::Seconds(TIdle);
        co_await ASC->UntilPoseFullyBlended(CuePose2, EPoseCue::Add);
    }
    
    const auto SpecTorque2 = MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, DeltaTheta > 0 ? GE_TorqueCW : GE_TorqueCCW);
    TorqueHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecTorque2);
    co_await Latent::Seconds(TTorque2);
    if(!RemoveActiveGameplayEffect(TorqueHandle, *ActorInfo, ActivationInfo))
        UE_LOGFMT(LogMyGame, Error, "Could not remove effect Torque1");

    co_await ASC->UntilPoseFullyBlended(CuePose2, EPoseCue::Remove);
    co_await ASC->UntilPoseFullyBlended(CuePose1, EPoseCue::Remove);
    
    ASC->RemoveGameplayCue(Tag.CueAccelerateShowThrusters);

    if(const auto OmegaLog = Cast<AMyPawn>(ActorInfo->OwnerActor)->GetOmega(); FMath::Abs(OmegaLog) <= 0.001)
        UE_LOGFMT(LogMyGame, Display, "Ability LookAt finished");
    else
        UE_LOGFMT(LogMyGame, Error, "Ability LookAt finished, but Omega > 0.001: Omega = {OMEGA}", OmegaLog);
    co_return;
}

void UGA_LookAt::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
    
    auto bRemoved = RemoveActiveGameplayEffect(TorqueHandle, *ActorInfo, ActivationInfo);
    UE_LOGFMT(LogMyGame, Warning, "Ability cancelled, gameplayeffect removed: {B}", bRemoved);
    Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}
