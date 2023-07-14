#pragma once

#include <functional>
#include <random>

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "Engine/LocalPlayer.h"

#include "MyState.generated.h"

class UMyGameInstance;
class ABlackhole;
DECLARE_LOG_CATEGORY_EXTERN(LogMyGame, All, All);

/*
 * everything stateful pertaining to the physics
 */
USTRUCT(BlueprintType)
struct FPhysics
{
	GENERATED_BODY()

	// scale factor for lengths = 1 UU / 1 m
	static constexpr double inline LengthScaleFactor = .01;

	// scale factor for mass, such that the astronaut weighs is set to roughly 160 (e.g. 1 = 1 kg)
	// this only affects displayed value as the physics are invariant to mass scale
	static constexpr double inline MassScaleFactor = 7./1000.;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	double WorldRadius = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	double WorldRadiusMeters = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector VecF1 = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, BlueprintReadwrite)
	double Alpha = 0.f;

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

	FHighlight(): Orbit(nullptr), Size(0) {}
	FHighlight(class AOrbit* InOrbit, double InSize) : Orbit(InOrbit), Size(InSize) {}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AOrbit* Orbit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double Size;
};

USTRUCT(BlueprintType)
struct FInstanceUI
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowAllTrajectories = false;

	FHighlight Selected;
	FHighlight Hovered;
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
 * @brief the standard gravitational parameter ALPHA [m^3/s^2] = G * M for different bodies
 */
constexpr double ALPHA_Game_SI = 8e2;

constexpr double ALPHA_Ceres_SI = 7e10;
constexpr double ALPHA_Moon_SI = 4.8e12;
constexpr double ALPHA_Earth_SI = 4e14;
constexpr double ALPHA_Sun_SI = 1.3e20;

// for actors, unreal guaranties sanity for values of x and y within [-1048535, 1048535]
constexpr double MAX_WORLDRADIUS_UU = 1.048535e6;

const FPhysics PhysicsEditorDefault =
	{  MAX_WORLDRADIUS_UU
	, MAX_WORLDRADIUS_UU * FPhysics::LengthScaleFactor // = 1.048535e4 = 10 km
	, FVector::Zero()
	, ALPHA_Game_SI / (FPhysics::LengthScaleFactor * FPhysics::LengthScaleFactor * FPhysics::LengthScaleFactor)
	};

constexpr FPlayerUI PlayerUIEditorDefault =
	{ //TODO
	};

const FInstanceUI InstanceUIEditorDefault;
	
/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UMyState : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	static UMyState* Get() { return GEngine->GetEngineSubsystem<UMyState>(); }
	
	UFUNCTION(BlueprintPure)
	FPhysics GetPhysics(const AMyGameState* GS);

	UFUNCTION(BlueprintPure)
	FPhysics GetPhysicsAny(const UObject* Object);

	UFUNCTION(BlueprintPure)
	FPlayerUI GetPlayerUI(const AMyPlayerState* PS);

	FPlayerUI GetPlayerUIAny(const UObject* Object, const FLocalPlayerContext& LPC);
	
	UFUNCTION(BlueprintPure)
	FInstanceUI GetInstanceUI(const UMyGameInstance* GI);

	UFUNCTION(BlueprintPure)
	FInstanceUI GetInstanceUIAny(const UObject* Object);

	void WithPhysics(const UObject* Object, const std::function<void(FPhysics&)> Func);
	
	void WithPlayerUI(const UObject* Object, const FLocalPlayerContext& LPC, const std::function<void(FPlayerUI&)> Func);

	void WithInstanceUI(const UObject* Object, const std::function<void(FInstanceUI&)> Func);

    /**
     * @brief constant factor to construct tangents for spline points
     */
    static constexpr double SplineToCircle = 1.6568542494923806; // 4. * (sqrt(2) - 1.);

	// private members
};
