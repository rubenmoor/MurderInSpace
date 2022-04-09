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
		this->ClearSplinePoints();
	}
	
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
	virtual void BeginPlay() override;
	
public:
	void UpdateOrbit(FVector VecR, FVector VecV, float MU);

	UFUNCTION()
	void UpdateOrbit(float MU);

    /**
     * @brief initialize a new Kepler orbit
     * @param VecF1 the focal point 1 in world space, the big gravitational body is here
     * @param Body  any mesh is attached to this scene root, which will move around
     */
	void Initialize(FVector _VecF1, USceneComponent* _Body);

	static constexpr float DefaultMU = 1e7;
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler")
	orbit Orbit;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	FVector VecF1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USceneComponent* Body;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SplineDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	float VelocityScalar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	FVector Velocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	float VelocityNormalized;

	// period in s
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler")
	float Period;

	// semi-major axis of elliptic orbit
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler")
	float A = 0;

	/**
	 * @brief constant factor to construct tangents for spline points
	 * //const float CBezierToCircle = 4.0 / 3 * (sqrt(2) - 1);
	 * supposedly the above is the exact formula c = 0.5522
	 * trying out in UE results in 1.65 being optimal
	 */
	static constexpr float SplineToCircle = 1.65;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
};
