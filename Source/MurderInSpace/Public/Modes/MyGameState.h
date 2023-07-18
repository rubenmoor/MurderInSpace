#pragma once

#include "CoreMinimal.h"
#include "MyState.h"
#include "GameFramework/GameState.h"
#include "AttributeSet.h"

#include "MyGameState.generated.h"

class ABlackhole;

USTRUCT(BlueprintType)
struct FMyAttributeMetaData : public FAttributeMetaData
{
	GENERATED_BODY()

	FMyAttributeMetaData()
	{
	}
	
	FMyAttributeMetaData(float InBaseValue, float InMinValue, float InMaxValue, FString InDerivedAttributeInfo, bool InBCanStack)
	{
		BaseValue = InBaseValue;
		MinValue = InMinValue;
		MaxValue = InMaxValue;
		DerivedAttributeInfo = InDerivedAttributeInfo;
		bCanStack = InBCanStack;
	}
};

UCLASS()
class MURDERINSPACE_API AMyGameState : public AGameState
{
	GENERATED_BODY()

	friend class UMyState;

	AMyGameState();
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableGyration = true;

	static AMyGameState* Get(const UWorld* World) { return World->GetGameState<AMyGameState>(); }

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double MinimumDamage = 1.;

	UFUNCTION(BlueprintPure)
	ABlackhole* GetBlackhole() const;
    
	// game world parameters to be edited in blueprint and to be used in game
	// TODO: move base values to datatable
	UPROPERTY(ReplicatedUsing=OnRep_Physics, EditAnywhere, BlueprintReadWrite)
	FPhysics RP_Physics = PhysicsEditorDefault;

	// TODO: read from csv file
	UPROPERTY(EditDefaultsOnly, Category="Gameplay Ability System")
	TObjectPtr<UDataTable> InitialAttributeValues;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AActor> BlackholeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double GyrationOmegaInitial = 0.1;

	// event handlers

	virtual void BeginPlay() override;

	#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
	#endif

	// replication
	
	UFUNCTION()
	void OnRep_Physics();

	// private methods
	void AddInitialAttributeValue(FName Name, const FMyAttributeMetaData& Data) const;
};
