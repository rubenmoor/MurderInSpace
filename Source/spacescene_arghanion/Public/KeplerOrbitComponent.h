// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "KeplerOrbitComponent.generated.h"

UENUM(BlueprintType)
enum class orbit : uint8
{
	CIRCLE UMETA(DisplayName="Circle"),
	ELLIPSE UMETA(DisplayName="Ellipse"),
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
    	this->SetClosedLoop(true, false);
	}
	
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

public:
	void UpdateOrbit(FVector VecR, FVector VecV);

    /**
     * @brief initialize a new Kepler orbit
     * @param VecF1 the focal point 1 in world space, the big gravitational body is here
     * @param Body  any mesh is attached to this scene root, which will move around
     */
	void Initialize(FVector _VecF1, USceneComponent* _Body);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	orbit Orbit;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector VecF1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USceneComponent* Body;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SplineDistance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float VelocityScalar;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector Velocity;

	// period in s
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Period;

	// semi-major axis of elliptic orbit
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float A = 0;

	/**
	 * @brief constant factor to construct tangents for spline points
	 * //const float CBezierToCircle = 4.0 / 3 * (sqrt(2) - 1);
	 * supposedly the above is the exact formula c = 0.5522
	 * trying out in UE results in 1.65 being optimal
	 */
	static constexpr float SplineToCircle = 1.65;
};
