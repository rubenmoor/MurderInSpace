#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"

#include "MyAttributeSetBase.generated.h"

USTRUCT(BlueprintType)
struct MURDERINSPACE_API FMyAttributeInitializationData : public FTableRowBase
{
	GENERATED_BODY()

	FMyAttributeInitializationData()
	{
	}
	
	FMyAttributeInitializationData(float InValue) : Value(InValue)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Attribute")
	float Value = 0.;
};

USTRUCT(BlueprintType)
struct MURDERINSPACE_API FMyAttributeRow
{
	GENERATED_BODY()
	
	FName RowName;
	FMyAttributeInitializationData RowData;

	FMyAttributeRow()
	{
	}
		
	FMyAttributeRow(FName InName, FMyAttributeInitializationData InData)
		: RowName(InName), RowData(InData)
	{
	}
};

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UMyAttributeSetBase : public UAttributeSet
{
	GENERATED_BODY()

	// override in child classes to provide initial value for attributes
	virtual TArray<FMyAttributeRow> GetAttributeInitialValueRows();

	// event handlers
	virtual void PostInitProperties() override;
};
