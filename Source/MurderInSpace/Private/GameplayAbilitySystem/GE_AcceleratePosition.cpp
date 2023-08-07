#include "GameplayAbilitySystem/GE_AcceleratePosition.h"
#include "MyGameplayTags.h"
#include "GameplayAbilitySystem/MyAttributes.h"

UGE_AcceleratePosition::UGE_AcceleratePosition()
{
    DurationPolicy = EGameplayEffectDurationType::Infinite;

    const auto& Tag = FMyGameplayTags::Get();
    GameplayCues.Add(FGameplayEffectCue(Tag.CueAccelerateShowThrusters, 0., 1.));

    InheritableOwnedTagsContainer.AddTag(Tag.PoseAccelerate);
}
