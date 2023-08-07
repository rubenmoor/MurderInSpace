#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

#include "MyDeveloperSettings.generated.h"

class UMyGameplayAbility;
class AHandThruster;
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
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Pawn initialization")
	TSoftObjectPtr<UDataTable> InitialAttributeValues;
	
	// this abilities will be given to the pawn in BeginPlay
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Pawn initialiazation")
	TArray<TSubclassOf<UMyGameplayAbility>> StartupAbilities;

	// the starter thruster for the astronaut
    UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category="Pawn initialization")
    TSubclassOf<AHandThruster> HandThrusterInitialClass;
    
	// for InitialAttributeValues
	static void AddRowUnlessExists(UDataTable* Table, FName RowName, const FTableRowBase& RowData);
};
