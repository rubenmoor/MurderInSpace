#include "GameplayAbilitySystem/GE_TorqueCCW.h"

#include "GameplayAbilitySystem/MyAttributes.h"

UGE_TorqueCCW::UGE_TorqueCCW()
{
    DurationPolicy = EGameplayEffectDurationType::HasDuration;
    
    Modifiers.Add
        (MkAttributeOverrideModifier
            ( UAttrSetAcceleration::GetTorqueAttribute()
            , UAttrSetAcceleration::GetTorqueMaxAttribute()
            , 1.
            )
        );
}
