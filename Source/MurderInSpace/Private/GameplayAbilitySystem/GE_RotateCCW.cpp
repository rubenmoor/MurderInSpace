#include "GameplayAbilitySystem/GE_RotateCCW.h"

#include "GameplayAbilitySystem/MyAttributes.h"

UGE_RotateCCW::UGE_RotateCCW()
{
    DurationPolicy = EGameplayEffectDurationType::Infinite;

    Modifiers.Add
        (MkAttributeOverrideModifier
            ( UAttrSetAcceleration::GetOmegaAttribute()
            , UAttrSetAcceleration::GetOmegaMaxAttribute()
            , 1.
            )
        );
}
