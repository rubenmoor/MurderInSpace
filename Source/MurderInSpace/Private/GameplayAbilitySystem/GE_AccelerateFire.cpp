#include "GameplayAbilitySystem/GE_AccelerateFire.h"

#include "MyGameplayTags.h"
#include "GameplayAbilitySystem/MyAttributes.h"

UGE_AccelerateFire::UGE_AccelerateFire()
{
    DurationPolicy = EGameplayEffectDurationType::Infinite;
    
    const auto& Tag = FMyGameplayTags::Get();
    GameplayCues.Add(FGameplayEffectCue(Tag.CueAccelerateFire, 0., 1.));

    Modifiers.Add
        (MkAttributeOverrideModifier
                ( UAttrSetAcceleration::GetAccelerationSIAttribute()
                , UAttrSetAcceleration::GetAccelerationSIMaxAttribute()
                )
        );
}
