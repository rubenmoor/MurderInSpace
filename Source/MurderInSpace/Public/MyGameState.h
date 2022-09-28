// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <random>

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MyGameState.generated.h"

/*
 * playing field parameters
 */
USTRUCT(BlueprintType)
struct FSpaceParams
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ScaleFactor;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float WorldRadius;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float WorldRadiusMeters;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector VecF1;
	
	UPROPERTY(EditAnywhere, BlueprintReadwrite)
	float Alpha;
};

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API AMyGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	/**
	 * @brief the standard gravitational parameter ALPHA [m^3/s^2] = G * M for different bodies
	 */
	static constexpr float ALPHA_Game_SI = 800;
	
	static constexpr float ALPHA_Ceres_SI = 7e10;
	static constexpr float ALPHA_Moon_SI = 4.8e12;
	static constexpr float ALPHA_Earth_SI = 4e14;
	static constexpr float ALPHA_Sun_SI = 1.3e20;

	// for actors, unreal guaranties sanity for values of x and y within [-1048535, 1048535]
	static constexpr float MAX_WORLDRADIUS_UU = 1.048535e6;

	// length [m] = length [UU] * scale factor
	static constexpr float DEFAULT_SCALEFACTOR = .01;

	// default values for the game world for the editor
	inline static const FSpaceParams DefaultSpaceParams = FSpaceParams
		{ DEFAULT_SCALEFACTOR
	    , MAX_WORLDRADIUS_UU
		, MAX_WORLDRADIUS_UU * DEFAULT_SCALEFACTOR // = 1.048535e4 = 10 km
		, FVector(0., 0., 0.)
		, ALPHA_Game_SI / (DEFAULT_SCALEFACTOR * DEFAULT_SCALEFACTOR * DEFAULT_SCALEFACTOR)
	    };

	UFUNCTION()
	FSpaceParams GetSpaceParams() const { return SpaceParams; }

	UFUNCTION()
	float GetInitialAngularVelocity();
	
protected:
	// game world parameters to be edited in blueprint and to be used in game
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSpaceParams SpaceParams = DefaultSpaceParams;

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
