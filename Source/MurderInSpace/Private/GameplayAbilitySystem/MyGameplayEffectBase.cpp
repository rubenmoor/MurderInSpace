// 


#include "GameplayAbilitySystem/MyGameplayEffectBase.h"

FGameplayModifierInfo UMyGameplayEffectBase::MkAttributeOverrideModifier(const FGameplayAttribute& Attribute,
    const FGameplayAttribute& BaseAttribute, float Coefficient)
{
    FAttributeBasedFloat AttributeBasedFloat;
    AttributeBasedFloat.BackingAttribute =
        FGameplayEffectAttributeCaptureDefinition
            ( BaseAttribute
            , EGameplayEffectAttributeCaptureSource::Source
            , false
            );
    AttributeBasedFloat.Coefficient = Coefficient;
    return
        { .Attribute         = Attribute
        , .ModifierOp        =  EGameplayModOp::Override
        , .ModifierMagnitude = AttributeBasedFloat
        };
}
