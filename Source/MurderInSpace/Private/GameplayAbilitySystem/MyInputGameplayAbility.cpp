#include "GameplayAbilitySystem/MyInputGameplayAbility.h"

void UMyInputGameplayAbility::BindOnRelease(std::function<void()> Callback)
{
    OnReleaseDelegate.BindLambda([=]
    {
        Callback();
        OnReleaseDelegate.Unbind();
    });
}

void UMyInputGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if(!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
    }
}
