#include "GameplayAbilitySystem/GE_MoveForward.h"

#include "GameplayAbilitySystem/MyAttributes.h"

UGE_MoveForward::UGE_MoveForward()
{
    DurationPolicy = EGameplayEffectDurationType::Infinite;

    Modifiers.Add
        (MkAttributeOverrideModifier
            ( UAttrSetAcceleration::GetForwardSpeedAttribute()
            , UAttrSetAcceleration::GetForwardSpeedMaxAttribute()
            , 1.
            )
        );
}
