#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

#include "MyDeveloperSettings.generated.h"

class UMyInputAction;
class UGameplayEffectsMap;
class UGameplayEffect;
/**
 * 
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Gameplay Constants"))
class MURDERINSPACE_API UMyDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Attributes initial values")
	TSoftObjectPtr<UDataTable> InitialAttributeValues;

	// for InitialAttributeValues
	static void AddRowUnlessExists(UDataTable* Table, FName RowName, const FTableRowBase& RowData);
};
