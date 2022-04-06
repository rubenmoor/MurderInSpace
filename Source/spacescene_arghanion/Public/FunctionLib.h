// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Orbit.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Components/SplineComponent.h"
#include "FunctionLib.generated.h"

/**
 * 
 */
UCLASS()
class SPACESCENE_ARGHANION_API UFunctionLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static Orbit GetOrbit(FVector R, FVector V);
	
	UFUNCTION(BlueprintPure, Category="Orbit")
	static TArray<FSplinePoint> CreatePointsCircle(FVector VecR, FVector VecH);
	
	UFUNCTION(BlueprintPure, Category="Orbit")
	static TArray<FSplinePoint> CreatePointsEllipsis(float A, FVector VecE, FVector VecH);
	
	UFUNCTION(BlueprintPure, Category="Orbit")
	static TArray<FSplinePoint> CreatePointsParabola(float P, FVector VecE, FVector VecH);
	
	UFUNCTION(BlueprintPure, Category="Orbit")
	static TArray<FSplinePoint> CreatePointsHyperbola();
	
	UFUNCTION(BlueprintPure, Category="Orbit")
	static FVector Eccentricity(FVector R, FVector V);

	UFUNCTION(BlueprintPure, Category="Orbit")
	static FVector FocusPoint2(float A, FVector E);

	UFUNCTION(BlueprintPure, Category="Orbit")
	static float SemiMajorAxis(FVector R, FVector V);

	UFUNCTION(BlueprintPure, Category="Orbit")
	static float SemiMinorAxis(float A, float ESquared);

	UFUNCTION(BlueprintPure, Category="Orbit")
	static float Period(float A);

	UFUNCTION(BlueprintPure, Category="Orbit")
	static float Velocity(float R, float A);

	UFUNCTION(BlueprintPure, Category="Orbit")
	static float Perimeter(float A, float B);
	
	/**
	 * @brief standard gravitational parameter mu = G M
	 */
	static constexpr float MU = 1e7;
	
	/**
	 * @brief constant factor to construct tangents for spline points
	 * //const float CBezierToCircle = 4.0 / 3 * (sqrt(2) - 1);
	 * supposedly the above is the exact formula c = 0.5522
	 * trying out in UE results in 1.65 being optimal
	 */
	static constexpr float SplineToCircle = 1.65;
};
