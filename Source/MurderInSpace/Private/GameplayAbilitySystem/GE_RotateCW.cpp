#include "GameplayAbilitySystem/GE_RotateCW.h"
#include "GameplayAbilitySystem/MyAttributes.h"

UGE_RotateCW::UGE_RotateCW()
{
    DurationPolicy = EGameplayEffectDurationType::Infinite;

    Modifiers.Add
        (MkAttributeOverrideModifier
            ( UAttrSetAcceleration::GetOmegaAttribute()
            , UAttrSetAcceleration::GetOmegaMaxAttribute()
            , -1.
            )
        );
}
