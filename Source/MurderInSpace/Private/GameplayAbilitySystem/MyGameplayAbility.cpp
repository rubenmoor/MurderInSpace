#include "GameplayAbilitySystem/MyGameplayAbility.h"

#include "GameplayAbilitySystem/MyAbilitySystemComponent.h"
#include "Spacebodies/MyCharacter.h"
#include "UE5Coro/LatentAwaiters.h"
#include "MyGameplayTags.h"

UMyGameplayAbility::UMyGameplayAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    
    const auto& Tag = FMyGameplayTags::Get();
    AbilityTags.AddTag(Tag.BlockingTurn);
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
        co_await Latent::Cancel();
    
    if(auto* OnBlockingAbilityEnded = TurnBlocked(Handle, ActorInfo))
        co_await Latent::UntilDelegate(*OnBlockingAbilityEnded);
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

FOnGameplayAbilityEnded* UMyGameplayAbility::TurnBlocked(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo)
{
    auto* ASC = UMyAbilitySystemComponent::Get(ActorInfo);
    const auto& Tag = FMyGameplayTags::Get();
    
    // get active ability, ignore active ability that is awaiting its turn
    auto Specs =
        ASC->GetActiveAbilities
            (&Tag.BlockingTurn.GetSingleTagContainer()
            , nullptr
            , { Handle, ASC->AbilityAwaitingTurn}
            );
    if(!Specs.IsEmpty())
    {
        check(Specs.Num() == 1)
        
        // replace other ability that was awaiting its turn
        ASC->CancelAbilityHandle(ASC->AbilityAwaitingTurn);
        ASC->AbilityAwaitingTurn = Handle;
        return &Specs[0].GetPrimaryInstance()->OnGameplayAbilityEnded;
    }
    return nullptr;
}
