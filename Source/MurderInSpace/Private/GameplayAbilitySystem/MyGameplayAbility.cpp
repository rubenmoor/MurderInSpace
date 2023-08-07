#include "GameplayAbilitySystem/MyGameplayAbility.h"

#include "Spacebodies/MyCharacter.h"
#include "UE5Coro/LatentAwaiters.h"

UMyGameplayAbility::UMyGameplayAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
}

void UMyGameplayAbility::LocallyControlledDo(const FGameplayAbilityActorInfo* ActorInfo,
    std::function<void(AMyCharacter*)> Func)
{
    if(ActorInfo->IsLocallyControlled())
    {
        Func(Cast<AMyCharacter>(ActorInfo->AvatarActor));
    }
}

FAbilityCoroutine UMyGameplayAbility::ExecuteAbility(FGameplayAbilitySpecHandle Handle,
                                                     const FGameplayAbilityActorInfo* ActorInfo, FGameplayAbilityActivationInfo ActivationInfo,
                                                     const FGameplayEventData* TriggerEventData)
{
    if(!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        co_await Latent::Cancel();
    }
    co_await UntilReleased();
}

Private::FLatentAwaiter UMyGameplayAbility::UntilReleased()
{
    return Latent::Until([this] 
        {
            if(bReleased)
            {
                bReleased = false;
                return true;
            }
           return false; 
        });
}
