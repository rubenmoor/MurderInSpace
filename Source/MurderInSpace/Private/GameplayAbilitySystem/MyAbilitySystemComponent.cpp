#include "GameplayAbilitySystem/MyAbilitySystemComponent.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystemInterface.h"
#include "GameplayCueManager.h"
#include "Actors/MyPawn.h"
#include "GameplayAbilitySystem/MyGameplayAbility.h"
#include "Modes/MyPlayerController.h"

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

void UMyAbilitySystemComponent::ExecuteGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters & GameplayCueParameters)
{
    UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::Executed, GameplayCueParameters);
}

void UMyAbilitySystemComponent::AddGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters & GameplayCueParameters)
{
    UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::OnActive, GameplayCueParameters);
    UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::WhileActive, GameplayCueParameters);
}

void UMyAbilitySystemComponent::RemoveGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters & GameplayCueParameters)
{
    UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::Removed, GameplayCueParameters);
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
