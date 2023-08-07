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
    // max value for acceleration in m / s^2
	UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_AccelerationSIMax)
	FGameplayAttributeData AccelerationSIMax;
	ATTRIBUTE_ACCESSORS(UAttrSetAcceleration, AccelerationSIMax)
	
    // currently applied value for acceleration in m / s^2
	UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_AccelerationSI)
	FGameplayAttributeData AccelerationSI;
	ATTRIBUTE_ACCESSORS(UAttrSetAcceleration, AccelerationSI)
	
	// max value for torque
	UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_TorqueMax)
	FGameplayAttributeData TorqueMax;
	ATTRIBUTE_ACCESSORS(UAttrSetAcceleration, TorqueMax)

	// currently applied value for torque
	UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_Torque)
	FGameplayAttributeData Torque;
	ATTRIBUTE_ACCESSORS(UAttrSetAcceleration, Torque)

	// maximal angular velocity
	UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_OmegaMax)
	FGameplayAttributeData OmegaMax;
	ATTRIBUTE_ACCESSORS(UAttrSetAcceleration, OmegaMax)

	UFUNCTION()
	virtual void OnRep_AccelerationSIMax(const FGameplayAttributeData& OldAccelerationSIMax);
	
	UFUNCTION()
	virtual void OnRep_AccelerationSI(const FGameplayAttributeData& OldAccelerationSI);
	
	UFUNCTION()
	virtual void OnRep_TorqueMax(const FGameplayAttributeData& OldTorqueMax);
	
	UFUNCTION()
	virtual void OnRep_Torque(const FGameplayAttributeData& OldTorque);
	
	UFUNCTION()
	virtual void OnRep_OmegaMax(const FGameplayAttributeData& OldOmegaMax);

private:
	virtual TArray<FMyAttributeRow> GetAttributeInitialValueRows() override;
};
