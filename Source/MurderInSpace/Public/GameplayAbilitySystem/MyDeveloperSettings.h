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

#if WITH_EDITOR
	UMyDeveloperSettings();
#endif
	
public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Attributes initial values")
	TSoftObjectPtr<UDataTable> InitialAttributeValues;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Input Action Assets")
	TArray<TSoftObjectPtr<UMyInputAction>> MyInputActions;

#if WITH_EDITOR
	UFUNCTION(CallInEditor)
	void ResetMyInputActions();
#endif
	
	// for InitialAttributeValues
	static void AddRowUnlessExists(UDataTable* Table, FName RowName, const FTableRowBase& RowData);
};
