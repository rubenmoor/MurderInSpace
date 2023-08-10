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
        auto* ASC = UMyAbilitySystemComponent::Get(&ActorInfo);
        if(ASC->GetActiveGameplayEffect(Handle))
            return ASC->RemoveActiveGameplayEffect(Handle);
    }
    return false;
}

Private::FLatentAwaiter UMyGameplayAbility::UntilReleased()
{
    return Private::FLatentAwaiter(&bReleased, [] (void* State, bool bCleanUp) -> bool
    {
        auto* ptrBReleased = static_cast<bool*>(State);
        if(bCleanUp)
            return false;
        if(*ptrBReleased)
        {
            *ptrBReleased = false;
            return true;
        }
        return false;
    });
}
