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

UMyAbilitySystemComponent* UMyAbilitySystemComponent::Get(const IAbilitySystemInterface* InPawn)
{
    return Cast<UMyAbilitySystemComponent>(InPawn->GetAbilitySystemComponent());
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

TArray<FGameplayAbilitySpec> UMyAbilitySystemComponent::GetActiveAbilities
    ( const FGameplayTagContainer* WithTags
    , const FGameplayTagContainer* WithoutTags
    , TArray<FGameplayAbilitySpecHandle> IgnoreList
    )
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

		if (WithTagPass && WithoutTagPass && !IgnoreList.Contains(Spec.Handle))
		{
			ActiveAbilities.Add(Spec);
		}
    }
    return ActiveAbilities;
}

void UMyAbilitySystemComponent::SendGameplayEvent(FGameplayTag EventTag, FGameplayEventData EventData)
{
    FScopedPredictionWindow NewScopedWindow(this, true);
    HandleGameplayEvent(EventTag, &EventData);
}

bool UMyAbilitySystemComponent::AddGameplayCueUnlessExists(FGameplayTag Cue)
{
    if(!HasMatchingGameplayTag(Cue))
    {
        AddGameplayCue(Cue);
        return true;
    }
    return false;
}

bool UMyAbilitySystemComponent::RemoveGameplayCueIfExists(FGameplayTag Cue)
{
    if(HasMatchingGameplayTag(Cue))
    {
        RemoveGameplayCue(Cue);
        return true;
    }
    return false;
}
