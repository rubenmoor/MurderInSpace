#include "GameplayAbilitySystem/MyAbilitySystemComponent.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystemInterface.h"
#include "GameplayCueManager.h"
#include "Spacebodies/MyPawn.h"
#include "GameplayAbilitySystem/MyGameplayAbility.h"
#include "UE5Coro/LatentAwaiters.h"

UMyAbilitySystemComponent::UMyAbilitySystemComponent()
{
    ReplicationMode = EGameplayEffectReplicationMode::Mixed;
}

UMyAbilitySystemComponent* UMyAbilitySystemComponent::Get(AMyPawn* InPawn)
{
    check(InPawn->Implements<UAbilitySystemInterface>())
    return Cast<UMyAbilitySystemComponent>(Cast<IAbilitySystemInterface>(InPawn)->GetAbilitySystemComponent());
}

UMyAbilitySystemComponent* UMyAbilitySystemComponent::Get(const FGameplayAbilityActorInfo* ActorInfo)
{
    return Cast<UMyAbilitySystemComponent>(ActorInfo->AbilitySystemComponent);
}

FGameplayTag UMyAbilitySystemComponent::FindTag(FGameplayTag InTag)
{
	FGameplayTagContainer MyTags;
	GetOwnedGameplayTags(MyTags);
	return MyTags.Filter(InTag.GetSingleTagContainer()).First();
}

TArray<FGameplayAbilitySpec> UMyAbilitySystemComponent::GetActiveAbilities(const FGameplayTagContainer* WithTags,
    const FGameplayTagContainer* WithoutTags, UGameplayAbility* Ignore)
{
    ABILITYLIST_SCOPE_LOCK()
    TArray<FGameplayAbilitySpec> ActiveAbilities;
    for(FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
    {
        if(!Spec.IsActive())
        {
            continue;
        }
		const bool WithTagPass = (!WithTags || Spec.Ability->AbilityTags.HasAny(*WithTags));
		const bool WithoutTagPass = (!WithoutTags || !Spec.Ability->AbilityTags.HasAny(*WithoutTags));

		if (WithTagPass && WithoutTagPass && Ignore != Spec.Ability)
		{
			ActiveAbilities.Add(Spec);
		}
    }
    return ActiveAbilities;
}

Private::FLatentAwaiter UMyAbilitySystemComponent::UntilPoseFullyBlended(FGameplayTag Cue, EPoseCue PoseCueChange)
{
    switch (PoseCueChange)
    {
    case EPoseCue::Add:
        if(!HasMatchingGameplayTag(Cue))
        {
            AddGameplayCue(Cue);
            return Latent::UntilDelegate(OnStateFullyBlended);
        }
    case EPoseCue::Remove:
        if(HasMatchingGameplayTag(Cue))
        {
            RemoveGameplayCue(Cue);
            return Latent::UntilDelegate(OnStateFullyBlended);
        }
    }
    return Private::FLatentAwaiter(nullptr, [] (void*, bool bCleanup){ return !bCleanup; });
}

void UMyAbilitySystemComponent::SendGameplayEvent(FGameplayTag EventTag, FGameplayEventData EventData)
{
    FScopedPredictionWindow NewScopedWindow(this, true);
    HandleGameplayEvent(EventTag, &EventData);
}

