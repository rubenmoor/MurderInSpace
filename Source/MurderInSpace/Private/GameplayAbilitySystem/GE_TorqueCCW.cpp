#include "GameplayAbilitySystem/GE_TorqueCCW.h"

#include "MyGameplayTags.h"
#include "GameplayAbilitySystem/MyAttributes.h"

UGE_TorqueCCW::UGE_TorqueCCW()
{
    DurationPolicy = EGameplayEffectDurationType::HasDuration;
    
    const auto& Tag = FMyGameplayTags::Get();
    GameplayCues.Add(FGameplayEffectCue(Tag.CueAccelerateFire, 0., 1.));
    
    Modifiers.Add
        (MkAttributeOverrideModifier
            ( UAttrSetAcceleration::GetTorqueAttribute()
            , UAttrSetAcceleration::GetTorqueMaxAttribute()
            , 1.
            )
        );
}
