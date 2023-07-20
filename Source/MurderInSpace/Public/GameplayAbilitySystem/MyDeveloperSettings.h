#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

#include "MyDeveloperSettings.generated.h"

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

	// put a gameplay effect with DurationPolicy infinite here
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Asset references")
	TSubclassOf<UGameplayEffect> GameplayEffectInfiniteClass;

	static void AddRowUnlessExists(UDataTable* Table, FName RowName, const FTableRowBase& RowData);
};
