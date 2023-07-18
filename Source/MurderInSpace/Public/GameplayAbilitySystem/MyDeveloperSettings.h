#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

#include "MyDeveloperSettings.generated.h"

struct FMyAttributeRow;

/**
 * 
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Gameplay Constants"))
class MURDERINSPACE_API UMyDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Attributes initial values", AdvancedDisplay)
	TSoftObjectPtr<UDataTable> InitialAttributeValues;

	static void AddRowUnlessExists(UDataTable* Table, FName RowName, const FTableRowBase& RowData);
};
