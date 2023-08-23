#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

#include "MyDeveloperSettings.generated.h"

class UMyGameplayAbility;
class AHandThruster;
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
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Pawn initialization")
	TSoftObjectPtr<UDataTable> InitialAttributeValues;
	
	// the starter thruster for the astronaut
    UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category="Pawn initialization")
    TSubclassOf<AHandThruster> HandThrusterInitialClass;
    
	// for InitialAttributeValues
	static void AddRowUnlessExists(UDataTable* Table, FName RowName, const FTableRowBase& RowData);
};
