#include "GameplayAbilitySystem/GE_MoveForward.h"

#include "MyGameplayTags.h"
#include "GameplayAbilitySystem/MyAttributes.h"

UGE_MoveForward::UGE_MoveForward()
{
    DurationPolicy = EGameplayEffectDurationType::Infinite;

    const auto& Tag = FMyGameplayTags::Get();
    GameplayCues.Add(FGameplayEffectCue(Tag.CueThrustersFire, 0., 1.));

    Modifiers.Add
        (MkAttributeOverrideModifier
            ( UAttrSetAcceleration::GetForwardSpeedAttribute()
            , UAttrSetAcceleration::GetForwardSpeedMaxAttribute()
            , 1.
            )
        );
}
