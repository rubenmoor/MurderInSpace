#include "GameplayAbilitySystem/MyGameplayAbility.h"

#include "Logging/StructuredLog.h"
#include "Modes/MyState.h"
#include "UE5Coro/Cancellation.h"
#include "UE5Coro/LatentAwaiters.h"

UMyGameplayAbility::UMyGameplayAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
}

FAbilityCoroutine UMyGameplayAbility::ExecuteAbility(FGameplayAbilitySpecHandle Handle,
                                                     const FGameplayAbilityActorInfo* ActorInfo, FGameplayAbilityActivationInfo ActivationInfo,
                                                     const FGameplayEventData* TriggerEventData)
{
    if(!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        co_await Latent::Cancel();
    }
    co_await Latent::Until([this] { return bReleased; });
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
