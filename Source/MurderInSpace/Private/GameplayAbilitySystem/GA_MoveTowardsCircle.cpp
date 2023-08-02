#include "GameplayAbilitySystem/GA_MoveTowardsCircle.h"

#include "MyGameplayTags.h"

UGA_MoveTowardsCircle::UGA_MoveTowardsCircle()
{
    const auto& Tag = FMyGameplayTags::Get();
    AbilityTags.AddTag(Tag.InputBindingAbilityMoveTowardsCircle);
    ActivationOwnedTags.AddTag(Tag.AccelerationMoveTowardsCircle);
}
