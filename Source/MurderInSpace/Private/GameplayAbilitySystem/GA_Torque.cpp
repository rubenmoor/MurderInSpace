#include "GameplayAbilitySystem/GA_Torque.h"

#include "GameplayAbilitySystem/MyAbilitySystemComponent.h"

UGA_Torque::UGA_Torque()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
}

void UGA_Torque::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if(!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
    }
}
