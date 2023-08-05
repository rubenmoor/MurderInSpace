#include "GameplayAbilitySystem/MyGameplayAbility.h"

#include "Actors/MyCharacter.h"
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
    co_await UntilReleased();
}

void UMyGameplayAbility::LocallyDo(const FGameplayAbilityActorInfo* ActorInfo, std::function<void(AMyCharacter*)> Func)
{
    if(Cast<APawn>(ActorInfo->OwnerActor)->IsLocallyControlled())
    {
        Func(Cast<AMyCharacter>(ActorInfo->OwnerActor));
    }
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
