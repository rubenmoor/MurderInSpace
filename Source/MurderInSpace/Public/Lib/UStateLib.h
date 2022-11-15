// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <random>

#include "CoreMinimal.h"
#include "UStateLib.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMyGame, All, All);

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
 * any state that is related to the player UI;
 * The player UI is specific to local players, for UI that is specific to the game instance,
 * see the instance UI
 */
USTRUCT(BlueprintType)
struct FPlayerUI
{
	GENERATED_BODY()
	
	// TODO
};

USTRUCT(BlueprintType)
struct FHighlight
{
	GENERATED_BODY()

	FHighlight(): Orbit(), Size(0) {}
	FHighlight(class AOrbit* InOrbit, float InSize) : Orbit(InOrbit), Size(InSize) {}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AOrbit* Orbit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Size;
};

USTRUCT(BlueprintType)
struct FInstanceUI
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowAllTrajectories;

	TOptional<FHighlight> Selected;
	TOptional<FHighlight> Hovered;
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
	friend class UMyGameInstance;
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

	inline static const FPlayerUI DefaultPlayerUI =
		{ // TODO
		};

	inline static const FInstanceUI DefaultInstanceUI =
		{ false
		, TOptional<FHighlight>()
		, TOptional<FHighlight>()
		};
public:
	UFUNCTION(BlueprintPure)
	static float GetInitialAngularVelocity(FRnd Rnd);

	UFUNCTION(BlueprintPure)
	static FPhysics GetPhysics(const AMyGameState* GS);

	UFUNCTION(BlueprintPure)
	static FPhysics GetPhysicsUnsafe(const UObject* Object);

	UFUNCTION(BlueprintPure)
	static FPhysics GetPhysicsEditorDefault();
	
	UFUNCTION(BlueprintPure)
	static FPlayerUI GetPlayerUI(const AMyPlayerState* PS);

	static FPlayerUI GetPlayerUIUnsafe(const UObject* Object, const FLocalPlayerContext& PC);
	
	UFUNCTION(BlueprintPure)
	static FPlayerUI GetPlayerUIEditorDefault();

	UFUNCTION(BlueprintPure)
	static FInstanceUI GetInstanceUI(const UMyGameInstance* GI);

	UFUNCTION(BlueprintPure)
	static FInstanceUI GetInstanceUIUnsafe(const UObject* Object);

	UFUNCTION(BlueprintPure)
	static FInstanceUI GetInstanceUIEditorDefault();

	UFUNCTION(BlueprintPure)
	static FRnd GetRnd(const AMyGameState* GS, const UMyGameInstance* GI);
	
	UFUNCTION(BlueprintPure)
	static FRnd GetRndUnsafe(const UObject* Object);

	static void WithPlayerUIUnsafe(const UObject* Object, const FLocalPlayerContext& LPC, const std::function<void(FPlayerUI&)> Func);

	static void WithInstanceUIUnsafe(const UObject* Object, const std::function<void(FInstanceUI&)> Func);
	
	// TODO
	// UFUNCTION(BlueprintCallable)
	// static void SetInstanceState(UMyGameInstance* GI, int32 PlayerNum, EInstanceState InNewState);
};