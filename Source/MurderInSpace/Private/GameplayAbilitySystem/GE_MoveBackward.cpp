#include "GameplayAbilitySystem/GE_MoveBackward.h"

#include "GameplayAbilitySystem/MyAttributes.h"

UGE_MoveBackward::UGE_MoveBackward()
{
    DurationPolicy = EGameplayEffectDurationType::Infinite;

    Modifiers.Add
        (MkAttributeOverrideModifier
            ( UAttrSetAcceleration::GetForwardSpeedAttribute()
            , UAttrSetAcceleration::GetForwardSpeedMaxAttribute()
            , -1.
            )
        );
}
