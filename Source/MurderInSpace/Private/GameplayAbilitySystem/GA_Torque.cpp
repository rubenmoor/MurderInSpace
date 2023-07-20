#include "GameplayAbilitySystem/GA_Torque.h"

#include "GameplayAbilitySystem/MyAbilitySystemComponent.h"
#include "GameplayAbilitySystem/MyGameplayTags.h"

void UGA_Torque::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                   const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if(!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
    }
    const auto Tag = FMyGameplayTags::Get();
    if(Tag)
    const auto TheTag = RotationDirection == ERotationDirection::Counterclockwise ? Tag.HasTorqueCCW : Tag.HasTorqueCW;
    UMyAbilitySystemComponent::Get(ActorInfo)->AddGameplayTag(TheTag);
}
