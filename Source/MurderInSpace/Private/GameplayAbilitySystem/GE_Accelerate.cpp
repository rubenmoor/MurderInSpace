#include "GameplayAbilitySystem/GE_Accelerate.h"
#include "MyGameplayTags.h"

UGE_Accelerate::UGE_Accelerate()
{
    DurationPolicy = EGameplayEffectDurationType::Infinite;

    const auto& Tag = FMyGameplayTags::Get();
    GameplayCues.Add(FGameplayEffectCue(Tag.CueAccelerate, 0., 1.));
}
