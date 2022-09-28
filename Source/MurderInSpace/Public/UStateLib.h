// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <random>

#include "CoreMinimal.h"
#include "MyGameInstance.h"
#include "UStateLib.generated.h"

/*
 * everything stateful pertaining to the physics
 */
USTRUCT(BlueprintType)
struct FPhysics
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

/*
 * any state that is related to the player UI
 */
USTRUCT(BlueprintType)
struct FPlayerUI
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowAllTrajectories = false;
};

USTRUCT(BlueprintType)
struct FRnd
{
	GENERATED_BODY()

	std::default_random_engine RndGen;
	std::poisson_distribution<int> Poisson;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FRandomStream Stream;
};

/**
 *  consolidate GameState and PlayerState to provide a read-only access to the entire state
 *
 *  for anything before BeginPlay, `State` provides editor defaults
 */
UCLASS()
class MURDERINSPACE_API UStateLib : public UObject
{
	GENERATED_BODY()

	friend class AMyGameState;
	friend class AMyPlayerState;
protected:
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
	inline static const FPhysics DefaultPhysics = FPhysics
		{ DEFAULT_SCALEFACTOR
	    , MAX_WORLDRADIUS_UU
		, MAX_WORLDRADIUS_UU * DEFAULT_SCALEFACTOR // = 1.048535e4 = 10 km
		, FVector(0., 0., 0.)
		, ALPHA_Game_SI / (DEFAULT_SCALEFACTOR * DEFAULT_SCALEFACTOR * DEFAULT_SCALEFACTOR)
	    };

	inline static constexpr FPlayerUI DefaultPlayerUI = FPlayerUI
		{ false
		};
public:
	UFUNCTION(BlueprintPure)
	static float GetInitialAngularVelocity(FRnd Rnd);

	UFUNCTION(BlueprintPure)
	static FPhysics GetPhysics(AMyGameState* GS);

	UFUNCTION(BlueprintPure)
	static FPhysics GetPhysicsUnsafe(const UObject* Object);

	UFUNCTION(BlueprintPure)
	static FPhysics GetPhysicsEditorDefault();
	
	UFUNCTION(BlueprintPure)
	static FPlayerUI GetPlayerUI(AMyPlayerState* PS);

	UFUNCTION(BlueprintPure)
	static FPlayerUI GetPlayerUIUnsafe(UObject* Object);
	
	UFUNCTION(BlueprintPure)
	static FPlayerUI GetPlayerUIEditorDefault();

	UFUNCTION(BlueprintPure)
	static FRnd GetRnd(AMyGameState* GS, UMyGameInstance* GI);
	
	UFUNCTION(BlueprintPure)
	static FRnd GetRndUnsafe(UObject* Object);

	static void WithPlayerUIUnsafe(const UObject* Object, const std::function<FPlayerUI(FPlayerUI)>& Func);
};