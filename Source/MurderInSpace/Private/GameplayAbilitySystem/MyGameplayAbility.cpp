#include "GameplayAbilitySystem/MyGameplayAbility.h"

#include "GameplayAbilitySystem/MyAbilitySystemComponent.h"
#include "Spacebodies/MyCharacter.h"
#include "UE5Coro/LatentAwaiters.h"

UMyGameplayAbility::UMyGameplayAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UMyGameplayAbility::ServerRPC_SetReleased_Implementation()
{
    bReleased = true;
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

bool UMyGameplayAbility::RemoveActiveGameplayEffect(FActiveGameplayEffectHandle Handle, const FGameplayAbilityActorInfo& ActorInfo, FGameplayAbilityActivationInfo ActivationInfo, int32 StacksToRemove)
{
    if(HasAuthority(&ActivationInfo))
    {
        return UMyAbilitySystemComponent::Get(&ActorInfo)->RemoveActiveGameplayEffect(Handle);
    }
    else
    {
        return false;
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
