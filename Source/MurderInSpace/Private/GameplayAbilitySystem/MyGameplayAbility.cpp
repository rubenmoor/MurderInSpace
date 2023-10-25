#include "GameplayAbilitySystem/MyGameplayAbility.h"

#include "GameplayAbilitySystem/MyAbilitySystemComponent.h"
#include "Spacebodies/MyCharacter.h"
#include "UE5Coro/LatentAwaiters.h"
#include "MyGameplayTags.h"
#include "Engine/LocalPlayer.h"
#include "HUD/MyHUD.h"
#include "HUD/UW_MyAbilities.h"
#include "Logging/StructuredLog.h"
#include "Modes/MyGameInstance.h"
#include "Modes/MyPlayerController.h"

UMyGameplayAbility::UMyGameplayAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    
    const auto& Tag = FMyGameplayTags::Get();
}

void UMyGameplayAbility::ServerRPC_SetReleased_Implementation()
{
    bReleased = true;
}

void UMyGameplayAbility::LocallyControlledDo(const FGameplayAbilityActorInfo* ActorInfo,
                                             std::function<void(const FLocalPlayerContext&)> Func)
{
    auto* PC = Cast<APlayerController>(Cast<AMyCharacter>(ActorInfo->AvatarActor)->GetController());
    if(ActorInfo->IsLocallyControlled() && IsValid(PC))
    {
        Func(FLocalPlayerContext(PC));
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
            return ASC->RemoveActiveGameplayEffect(Handle, StacksToRemove);
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
    
    // get active ability;
    // ignore active ability that is awaiting its turn,
    // ignore this ability (this ability called TurnBlocked)
    FGameplayAbilitySpec* CurrentSpec = ASC->FindAbilitySpecFromHandle(InHandle);
    auto Specs =
        ASC->GetActiveInstancedPerActorAbilities
            ( Tag.BlockingTurn.GetSingleTagContainer()
            , FGameplayTagContainer{}
            , { CurrentSpec, AwaitingTurnSpec }
            );

    // no active ability, turn is not blocked
    if(Specs.IsEmpty())
        return nullptr;
    
    check(Specs.Num() == 1)

    // replace other ability that was awaiting its turn
    if(AwaitingTurnSpec)
    {
        check(AwaitingTurnSpec->IsActive())
        if(this != AwaitingTurnSpec->GetPrimaryInstance())
        {
            ASC->CancelAbilitySpec(*AwaitingTurnSpec, nullptr);
            LocallyControlledDo(ActorInfo, [this, AwaitingTurnSpec, &Tag] (const FLocalPlayerContext& LPC)
            {
                LPC.GetHUD<AMyHUD>()->WidgetHUD->WidgetAbilities->SetBordered
                    (AwaitingTurnSpec->Ability->AbilityTags.Filter(Tag.Ability.GetSingleTagContainer()).First()
                    , false
                    );
            });
        }
    }
    
    ASC->SetAbilityAwaitingTurn(*CurrentSpec);
    auto* ActiveAbility = Cast<UMyGameplayAbility>(Specs[0]->GetPrimaryInstance());
    auto* OnMyAbilityEnded = ActiveAbility->GetCustomOnAbilityEnded();
    OnGameplayAbilityEndedHandle = OnMyAbilityEnded->AddLambda([this, ASC] (UGameplayAbility* Ability)
    {
        ASC->ClearAbilityAwaitingTurn();
        Cast<UMyGameplayAbility>(Ability)->GetCustomOnAbilityEnded()->Remove(OnGameplayAbilityEndedHandle);
    });
    return OnMyAbilityEnded;
}
