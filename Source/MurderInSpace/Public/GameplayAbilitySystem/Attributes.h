#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"

#include "Attributes.generated.h"

// Uses macros from AttributeSet.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UAttrSetTorque : public UAttributeSet
{
	GENERATED_BODY()

public:	
//protected:
	UPROPERTY(BlueprintReadOnly, Category = "Torque", ReplicatedUsing = OnRep_Torque)
	FGameplayAttributeData Torque;
	ATTRIBUTE_ACCESSORS(UAttrSetTorque, Torque)

	// maximal angular velocity
	UPROPERTY(BlueprintReadOnly, Category = "Torque", ReplicatedUsing = OnRep_OmegaMax)
	FGameplayAttributeData OmegaMax;
	ATTRIBUTE_ACCESSORS(UAttrSetTorque, OmegaMax)

	UFUNCTION()
	virtual void OnRep_Torque(const FGameplayAttributeData& OldTorque);
	
	UFUNCTION()
	virtual void OnRep_OmegaMax(const FGameplayAttributeData& OldOmegaMax);

	// event handlers

	virtual void PostInitProperties() override;
};
