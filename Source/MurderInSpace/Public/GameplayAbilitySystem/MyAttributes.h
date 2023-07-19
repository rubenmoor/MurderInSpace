#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "MyAttributeSetBase.h"

#include "MyAttributes.generated.h"

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
class MURDERINSPACE_API UAttrSetAcceleration : public UMyAttributeSetBase
{
	GENERATED_BODY()

public:
    // Acceleration in m / s^2
	UPROPERTY(BlueprintReadOnly, Category = "TorqueMax", ReplicatedUsing = OnRep_TorqueMax)
	FGameplayAttributeData AccelerationSIMax;
	ATTRIBUTE_ACCESSORS(UAttrSetAcceleration, AccelerationSIMax)
	
	// torque when rotating
	UPROPERTY(BlueprintReadOnly, Category = "TorqueMax", ReplicatedUsing = OnRep_TorqueMax)
	FGameplayAttributeData TorqueMax;
	ATTRIBUTE_ACCESSORS(UAttrSetAcceleration, TorqueMax)

	// maximal angular velocity
	UPROPERTY(BlueprintReadOnly, Category = "Torque", ReplicatedUsing = OnRep_OmegaMax)
	FGameplayAttributeData OmegaMax;
	ATTRIBUTE_ACCESSORS(UAttrSetAcceleration, OmegaMax)

	UFUNCTION()
	virtual void OnRep_AccelerationSIMax(const FGameplayAttributeData& OldTorqueMax);
	
	UFUNCTION()
	virtual void OnRep_TorqueMax(const FGameplayAttributeData& OldTorqueMax);
	
	UFUNCTION()
	virtual void OnRep_OmegaMax(const FGameplayAttributeData& OldOmegaMax);

private:
	virtual TArray<FMyAttributeRow> GetAttributeInitialValueRows() override;
};
