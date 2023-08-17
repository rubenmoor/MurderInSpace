#include "GameplayAbilitySystem/MyGameplayAbility.h"

#include "GameplayAbilitySystem/MyAbilitySystemComponent.h"
#include "Spacebodies/MyCharacter.h"
#include "UE5Coro/LatentAwaiters.h"
#include "MyGameplayTags.h"
#include "Logging/StructuredLog.h"
#include "Modes/MyGameInstance.h"

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

FOnGameplayAbilityEnded* UMyGameplayAbility::TurnBlocked(FGameplayAbilitySpecHandle InHandle, const FGameplayAbilityActorInfo* ActorInfo)
{
    auto* ASC = UMyAbilitySystemComponent::Get(ActorInfo);
    const auto& Tag = FMyGameplayTags::Get();
    auto* AwaitingTurnSpec = ASC->GetAbilityAwaitingTurn();
    
    // get active ability; ignore active ability that is awaiting its turn, ignore this ability
    FGameplayAbilitySpec* CurrentSpec = ASC->FindAbilitySpecFromHandle(InHandle);
    auto Specs =
        ASC->GetActiveAbilities
            ( Tag.BlockingTurn.GetSingleTagContainer()
            , FGameplayTagContainer()
            , { CurrentSpec, AwaitingTurnSpec }
            );
    if(!Specs.IsEmpty())
    {
        check(Specs.Num() == 1)

        UE_LOGFMT(LogMyGame, Display, "Turn blocked by {NAME}, other ability waiting: {NAMEWAITING}"
            , Specs[0]->GetPrimaryInstance()->GetFName()
            , AwaitingTurnSpec ? AwaitingTurnSpec->GetPrimaryInstance()->GetFName() : NAME_None
            );
        
        // replace other ability that was awaiting its turn
        if(AwaitingTurnSpec)
        {
            checkf(AwaitingTurnSpec->IsActive(), TEXT("OnCancelled resets AbilityAwaitingTurn to nullptr"))
            check(this != AwaitingTurnSpec->GetPrimaryInstance())
            ASC->CancelAbilitySpec(*AwaitingTurnSpec, nullptr);
        }
        
        ASC->SetAbilityAwaitingTurn(*CurrentSpec);
        auto* ActiveAbility = Specs[0]->GetPrimaryInstance();
        OnGameplayAbilityEndedHandle = ActiveAbility->OnGameplayAbilityEnded.AddLambda([this, ASC] (UGameplayAbility* Ability)
        {
            ASC->ClearAbilityAwaitingTurn();
            Ability->OnGameplayAbilityEnded.Remove(OnGameplayAbilityEndedHandle);
        });
        return &ActiveAbility->OnGameplayAbilityEnded;
    }
    return nullptr;
}
