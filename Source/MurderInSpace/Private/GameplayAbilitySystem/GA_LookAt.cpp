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

    // TODO: not in use, so maybe delete?
    AbilityTags.AddTag(Tag.InputBindingAbilityLookAt);
    
    // cancel any active LookAt ability
    CancelAbilitiesWithTag.AddTag(Tag.AbilityLookAt);

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
    const float ThetaMin = OmegaMax * (OmegaMax / AlphaMax + 2 * TransitionTime);
    const float AngleDelta = TriggerEventData->EventMagnitude;
    const float AngleDeltaAbs = FMath::Abs(AngleDelta);

    const auto SpecTorque1 = ASC->MakeOutgoingSpec(AngleDelta > 0 ? GE_TorqueCCW : GE_TorqueCW, 1., MakeEffectContext(Handle, ActorInfo));
    const auto SpecTorque2 = ASC->MakeOutgoingSpec(AngleDelta > 0 ? GE_TorqueCW : GE_TorqueCCW, 1., MakeEffectContext(Handle, ActorInfo));

    float TTorque;
    float TRotatingAtAlphaZero;
    ASC->AddGameplayCue(Tag.CueAccelerateShowThrusters);
    if(AngleDeltaAbs > ThetaMin)
    {
        TTorque = AlphaMax / OmegaMax;
        TRotatingAtAlphaZero = AngleDeltaAbs / OmegaMax;
    }
    else
    {
        TTorque = FMath::Sqrt(AngleDeltaAbs / AlphaMax + FMath::Pow(TransitionTime, 2)) - TransitionTime;
        TRotatingAtAlphaZero = 2. * TransitionTime;
    }
    
    SpecTorque1.Data.Get()->SetDuration(TTorque, true);
    SpecTorque2.Data.Get()->SetDuration(TTorque, true);

    const FGameplayTag CuePose1 = AngleDelta > 0 ? Tag.CuePoseTurnCCW : Tag.CuePoseTurnCW;
    ASC->AddGameplayCue(CuePose1);
    co_await Latent::UntilDelegate(ASC->OnStateFullyBlended);
    ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecTorque1);
    co_await Latent::Seconds(TTorque);
    ASC->RemoveGameplayCue(CuePose1);

    co_await Latent::Seconds(TRotatingAtAlphaZero);
    
    const FGameplayTag CuePose2 = AngleDelta > 0 ? Tag.CuePoseTurnCW : Tag.CuePoseTurnCCW;
    ASC->AddGameplayCue(CuePose2);
    co_await Latent::UntilDelegate(ASC->OnStateFullyBlended);
    ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecTorque2);
    co_await Latent::Seconds(TTorque);
    ASC->RemoveGameplayCue(CuePose2);
    
    co_await Latent::UntilDelegate(ASC->OnStateFullyBlended);
    ASC->RemoveGameplayCue(Tag.CueAccelerateShowThrusters);
}
