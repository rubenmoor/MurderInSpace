// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <random>

#include "CoreMinimal.h"
#include "MyState.h"
#include "GameFramework/GameState.h"
#include "MyGameState.generated.h"

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API AMyGameState : public AGameState
{
	GENERATED_BODY()

	friend class UMyState;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableGyration = true;
	
protected:

	// game world parameters to be edited in blueprint and to be used in game
	UPROPERTY(ReplicatedUsing=OnRep_Physics, EditAnywhere, BlueprintReadWrite)
	FPhysics RP_Physics = PhysicsEditorDefault;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double GyrationOmegaInitial = 0.1;

	std::default_random_engine RndGen;
	std::poisson_distribution<int> Poisson;

	// event handlers

	virtual void PostInitializeComponents() override;

	#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
	#endif

	// replication
	
	UFUNCTION()
	void OnRep_Physics();
};
