// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "MyGameplayEffectBase.generated.h"

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UMyGameplayEffectBase : public UGameplayEffect
{
    GENERATED_BODY()

protected:
    FGameplayModifierInfo MkAttributeOverrideModifier(const FGameplayAttribute& Attribute, const FGameplayAttribute& BaseAttribute, float Coefficient = 1.);
};
