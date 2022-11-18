// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <random>

#include "MyGameInstance.h"
#include "MyGameState.h"
#include "MyPlayerState.h"
#include "Subsystems/EngineSubsystem.h"
#include "MyState.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMyGame, All, All);

/*
 * everything stateful pertaining to the physics
 */
USTRUCT(BlueprintType)
struct FPhysics
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ScaleFactor = 0.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float WorldRadius = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float WorldRadiusMeters = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector VecF1 = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, BlueprintReadwrite)
	float Alpha = 0.f;
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
 * 
 */
UCLASS()
class MURDERINSPACE_API UMyState : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * @brief the standard gravitational parameter ALPHA [m^3/s^2] = G * M for different bodies
	 */
	constexpr float ALPHA_Game_SI = 800;
	
	constexpr float ALPHA_Ceres_SI = 7e10;
	constexpr float ALPHA_Moon_SI = 4.8e12;
	constexpr float ALPHA_Earth_SI = 4e14;
	constexpr float ALPHA_Sun_SI = 1.3e20;

	// for actors, unreal guaranties sanity for values of x and y within [-1048535, 1048535]
	constexpr float MAX_WORLDRADIUS_UU = 1.048535e6;

	// length [m] = length [UU] * scale factor
	constexpr float DEFAULT_SCALEFACTOR = .01;

	UFUNCTION(BlueprintCallable)
	FPhysics GetPhysicsEditorDefault()
	{
		return
			{ DEFAULT_SCALEFACTOR
			, MAX_WORLDRADIUS_UU
			, MAX_WORLDRADIUS_UU * DEFAULT_SCALEFACTOR // = 1.048535e4 = 10 km
			, FVector(0., 0., 0.)
			, ALPHA_Game_SI / (DEFAULT_SCALEFACTOR * DEFAULT_SCALEFACTOR * DEFAULT_SCALEFACTOR)
			};
	}
	
	UFUNCTION(BlueprintCallable)
	FPlayerUI GetPlayerUIEditorDefault()
	{
		return FPlayerUI
			{ //TODO
			};
	}

	UFUNCTION(BlueprintCallable)
	FInstanceUI GetInstanceUIEditorDefault()
	{
		return
			{ false
			, TOptional<FHighlight>()
			, TOptional<FHighlight>()
			};
	}
	
	UFUNCTION(BlueprintPure)
	static FPhysics GetPhysics(const AMyGameState* GS)
	{
		return GS->Physics;
	}

	UFUNCTION(BlueprintPure)
	static FPhysics GetPhysicsAny(const UObject* Object);

	UFUNCTION(BlueprintPure)
	static FPlayerUI GetPlayerUI(const AMyPlayerState* PS)
	{
		return PS->PlayerUI;
	}

	static FPlayerUI GetPlayerUIAny(const UObject* Object, const FLocalPlayerContext& LPC);
	
	UFUNCTION(BlueprintPure)
	static FInstanceUI GetInstanceUI(const UMyGameInstance* GI)
	{
		return GI->InstanceUI;
	}

	UFUNCTION(BlueprintPure)
	static FInstanceUI GetInstanceUIAny(const UObject* Object);

	UFUNCTION(BlueprintPure)
	static FRnd GetRnd(const AMyGameState* GS, const UMyGameInstance* GI)
	{
		return FRnd
			{ GS->RndGen
			, GS->Poisson
			, GI->Random
			};
	}
	
	UFUNCTION(BlueprintPure)
	static FRnd GetRndAny(const UObject* Object);

	void WithPlayerUI(const UObject* Object, const FLocalPlayerContext& LPC, const std::function<void(FPlayerUI&)> Func);

	void WithInstanceUI(const UObject* Object, const std::function<void(FInstanceUI&)> Func);
	
	UFUNCTION(BlueprintPure)
	float GetInitialAngularVelocity(FRnd Rnd);

};
