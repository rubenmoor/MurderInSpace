#include "GameplayAbilitySystem/MyAbilitySystemComponent.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystemInterface.h"
#include "GameplayCueManager.h"
#include "MyGameplayTags.h"
#include "GameplayAbilitySystem/MyGameplayAbility.h"

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
    ( const FGameplayTagContainer& WithAnyTag
    , const FGameplayTagContainer& WithoutTags
    , TArray<FGameplayAbilitySpecHandle> IgnoreList
    )
{
    ABILITYLIST_SCOPE_LOCK()
    TArray<FGameplayAbilitySpec> ActiveAbilities;
    for(FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
    {
        if(Spec.IsActive())
        {
            const bool WithTagPass = WithAnyTag.IsEmpty() || Spec.Ability->AbilityTags.HasAny(WithAnyTag);
            const bool WithoutTagPass = WithoutTags.IsEmpty() || !Spec.Ability->AbilityTags.HasAny(WithoutTags);

            if (WithTagPass && WithoutTagPass && !IgnoreList.Contains(Spec.Handle))
            {
                ActiveAbilities.Add(Spec);
            }
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
    const auto& Tag = FMyGameplayTags::Get();
    checkf(!Cue.MatchesTag(Tag.CuePose), TEXT("Use AddPoseCue to properly handle bAnimStateFullyBlended"))
    if(!HasMatchingGameplayTag(Cue))
    {
        AddGameplayCue(Cue);
        return true;
    }
    return false;
}

bool UMyAbilitySystemComponent::RemoveGameplayCueIfExists(FGameplayTag Cue)
{
    const auto& Tag = FMyGameplayTags::Get();
    checkf(!Cue.MatchesTag(Tag.CuePose), TEXT("Use RemovePoseCue to properly handle bAnimStateFullyBlended"))
    if(HasMatchingGameplayTag(Cue))
    {
        RemoveGameplayCue(Cue);
        return true;
    }
    return false;
}

bool UMyAbilitySystemComponent::AddPoseCue(FGameplayTag PoseCue)
{
    const auto& Tag = FMyGameplayTags::Get();
    check(PoseCue.MatchesTag(Tag.CuePose))
    if(!HasMatchingGameplayTag(PoseCue))
    {
        AddGameplayCue(PoseCue);
        bAnimStateFullyBlended = false;
        return true;
    }
    return !bAnimStateFullyBlended;
}

bool UMyAbilitySystemComponent::RemovePoseCue(FGameplayTag PoseCue)
{
    const auto& Tag = FMyGameplayTags::Get();
    check(PoseCue.MatchesTag(Tag.CuePose))
    if(HasMatchingGameplayTag(PoseCue))
    {
        RemoveGameplayCue(PoseCue);
        bAnimStateFullyBlended = false;
        return true;
    }
    return !bAnimStateFullyBlended;
}

bool UMyAbilitySystemComponent::HasPose(FGameplayTag PoseCue) const
{
    const auto& Tag = FMyGameplayTags::Get();
    check(PoseCue.MatchesTag(Tag.CuePose))
    return HasMatchingGameplayTag(PoseCue) && bAnimStateFullyBlended;
}
