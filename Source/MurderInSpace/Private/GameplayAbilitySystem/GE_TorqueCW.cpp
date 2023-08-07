#include "GameplayAbilitySystem/GE_TorqueCW.h"

#include "GameplayAbilitySystem/MyAttributes.h"

UGE_TorqueCW::UGE_TorqueCW()
{
    DurationPolicy = EGameplayEffectDurationType::HasDuration;
    
    Modifiers.Add
        (MkAttributeOverrideModifier
            ( UAttrSetAcceleration::GetTorqueAttribute()
            , UAttrSetAcceleration::GetTorqueMaxAttribute()
            , -1.
            )
        );
}
