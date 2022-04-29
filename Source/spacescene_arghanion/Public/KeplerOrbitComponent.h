// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "KeplerOrbitComponent.generated.h"

UENUM(BlueprintType)
enum class orbit : uint8
{
	CIRCLE UMETA(DisplayName="Circle"),
	ELLIPSE UMETA(DisplayName="Ellipse"),
	LINEBOUND UMETA(DisplayName="LineBound"),
	LINEUNBOUND UMETA(DisplayName="LineUnbound"),
	PARABOLA UMETA(DisplayName="Parabola"),
	HYPERBOLA UMETA(DisplayName="Hyperbola")
};

/**
 * 
 */
UCLASS()
class SPACESCENE_ARGHANION_API UKeplerOrbitComponent : public USplineComponent
{
	GENERATED_BODY()

	UKeplerOrbitComponent()
	{
    	PrimaryComponentTick.bCanEverTick = true;
		ClearSplinePoints();
		bInitialized = false;
	}

	// I don't know how to pass arguments to the constructor, as Unreal somehow requires the default constructor for
	// CreateDefaultSubObject; the work-around is a seperate method `Initialize`; not calling it causes failure
	bool bInitialized;

	// Objects in the editor have their orbit pre-configured; however, objects spawned later need sensible defaults,
	// but the circular orbit depends on the location of the body, which isn't known before BeginPlay;
	// in order not to override orbits that are already set up, the defaults will be applied only when this is `false`: 
	bool bHasOrbit;
	
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
	
public:
	void UpdateOrbit(FVector VecR, FVector VecV, float MU, float RMAX);

	UFUNCTION()
	void UpdateOrbit(float MU, float RMAX);

    /**
     * @brief initialize a new Kepler orbit
     * @param _VecF1 the focal point 1 in world space, the big gravitational body is here
     * @param _Body  any mesh is attached to this scene root, which will move around
     */
	void Initialize(FVector _VecF1, TObjectPtr<USceneComponent> _Body, TObjectPtr<UHierarchicalInstancedStaticMeshComponent> _TrajectoryMesh);

	static constexpr float DefaultMU = 1e7;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler")
	orbit Orbit;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler" )
	float Eccentricity;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler", DisplayName="P = (H * H)/MU")
	float P;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler" )
	float Energy;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	FVector VecF1;

	TObjectPtr<USceneComponent> Body;
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> TrajectoryMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	float Velocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	FVector VecVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	float VelocityNormalized;

	// period in s
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler")
	float Period;

	// semi-major axis of elliptic orbit
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler")
	float A = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler")
	float SplineDistanceLineBound;
	
	/**
	 * @brief constant factor to construct tangents for spline points
	 * //const float CBezierToCircle = 4.0 / 3 * (sqrt(2) - 1);
	 * supposedly the above is the exact formula c = 0.5522
	 * trying out in UE results in 1.65 being optimal
	 */
	static constexpr float SplineToCircle = 1.65;

	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedChainEvent) override;
	virtual void BeginPlay() override;
};
