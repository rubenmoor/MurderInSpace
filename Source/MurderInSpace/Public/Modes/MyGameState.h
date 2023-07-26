#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"

#include "MyGameState.generated.h"

class ABlackhole;

/*
 * everything stateful pertaining to the physics
 */
USTRUCT(BlueprintType)
struct MURDERINSPACE_API FPhysics
{
	GENERATED_BODY()
	
	FPhysics()
		: WorldRadius(MAX_WORLDRADIUS_UU)
		, WorldRadiusMeters(MAX_WORLDRADIUS_UU * LengthScaleFactor) // = 1.048535e4 = 10 km
		, VecF1(FVector::Zero())
		, Alpha(ALPHA_Game_SI / (LengthScaleFactor * LengthScaleFactor * LengthScaleFactor))
	{
	}
	
	/**
	 * @brief the standard gravitational parameter ALPHA [m^3/s^2] = G * M for different bodies
	 */
	static constexpr double ALPHA_Game_SI = 8e2;

	static constexpr double ALPHA_Ceres_SI = 7e10;
	static constexpr double ALPHA_Moon_SI = 4.8e12;
	static constexpr double ALPHA_Earth_SI = 4e14;
	static constexpr double ALPHA_Sun_SI = 1.3e20;

	// for actors, unreal guaranties sanity for values of x and y within [-1048535, 1048535]
	static constexpr double MAX_WORLDRADIUS_UU = 1.048535e6;
	
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

UCLASS()
class MURDERINSPACE_API AMyGameState : public AGameState
{
	GENERATED_BODY()

	friend class UMyState;

public:
	static AMyGameState* Get(const UObject* Object)
	{
		return Cast<AMyGameState>(Object->GetWorld()->GetGameState());
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableGyration = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double MinimumDamage = 1.;

	UFUNCTION(BlueprintPure)
	ABlackhole* GetBlackhole() const;
    
	// game world parameters to be edited in blueprint and to be used in game
	// TODO: move base values to datatable
	UPROPERTY(ReplicatedUsing=OnRep_Physics, EditAnywhere, BlueprintReadWrite)
	FPhysics RP_Physics; // = FPhysics::EditorDefaults;

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
};
