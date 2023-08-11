#include "GameplayAbilitySystem/GE_TorqueCCW.h"

#include "MyGameplayTags.h"
#include "GameplayAbilitySystem/MyAttributes.h"

UGE_TorqueCCW::UGE_TorqueCCW()
{
    DurationPolicy = EGameplayEffectDurationType::Infinite;

    const auto& Tag = FMyGameplayTags::Get();
    GameplayCues.Add(FGameplayEffectCue(Tag.CueThrustersFire, 0., 1.));

    Modifiers.Add
        (MkAttributeOverrideModifier
            ( UAttrSetAcceleration::GetTorqueAttribute()
            , UAttrSetAcceleration::GetTorqueMaxAttribute()
            , 1.
            )
        );
}
