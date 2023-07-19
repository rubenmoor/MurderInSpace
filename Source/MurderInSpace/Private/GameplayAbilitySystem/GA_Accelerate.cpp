


#include "GameplayAbilitySystem/GA_Accelerate.h"

#include "GameplayAbilitySystem/MyGameplayTags.h"

UGA_Accelerate::UGA_Accelerate()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
    // TODO
    //ActivationBlockedTags =
    AbilityTags.AddTag(FMyGameplayTags::Get().Accelerate);
}

void UGA_Accelerate::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                     const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, MakeOutgoingGameplayEffectSpec(GameplayEffect));
}

void UGA_Accelerate::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
    const auto Tag = FMyGameplayTags::Get();
    BP_RemoveGameplayEffectFromOwnerWithGrantedTags(FGameplayTagContainer(Tag.IsAccelerating));
}

void UGA_Accelerate::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
    Super::OnAvatarSet(ActorInfo, Spec);
    check(IsValid(GameplayEffect))
}
