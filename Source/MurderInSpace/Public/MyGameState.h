// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <random>

#include "CoreMinimal.h"
#include "UStateLib.h"
#include "GameFramework/GameState.h"
#include "MyGameState.generated.h"

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API AMyGameState : public AGameState
{
	GENERATED_BODY()

	friend class UStateLib;

protected:

	// game world parameters to be edited in blueprint and to be used in game
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPhysics Physics = UStateLib::DefaultPhysics;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AngularVelocityPoissonMean = 1.;

	std::default_random_engine RndGen;
	std::poisson_distribution<int> Poisson;

	// event handlers

	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;

	#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
	#endif

private:
	// private methods
	void UpdateAllOrbits() const;
};
