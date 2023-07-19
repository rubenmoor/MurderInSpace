#include "GameplayAbilitySystem/MyAbilitySystemComponent.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystemInterface.h"
#include "GameplayCueManager.h"
#include "Actors/MyPawn.h"

UMyAbilitySystemComponent* UMyAbilitySystemComponent::Get(AMyPawn* InPawn)
{
    check(InPawn->Implements<UAbilitySystemInterface>())
    return Cast<UMyAbilitySystemComponent>(Cast<IAbilitySystemInterface>(InPawn)->GetAbilitySystemComponent());
}

void UMyAbilitySystemComponent::ApplyGE_MoveTowardsCircle()
{
    GE_MoveTowardsCircleHandle = ApplyGameplayEffectSpecToSelf( *MakeOutgoingSpec(GE_MoveTowardsCircle, 1.0, MakeEffectContext()).Data.Get());
}

void UMyAbilitySystemComponent::RemoveGE_MoveTowardsCircle()
{
    RemoveActiveGameplayEffect(GE_MoveTowardsCircleHandle);
}

void UMyAbilitySystemComponent::BeginPlay()
{
    Super::BeginPlay();
    check(IsValid(GE_MoveTowardsCircle))
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