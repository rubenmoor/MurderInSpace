// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FunctionLib.generated.h"

/**
 * 
 */
UCLASS()
class SPACESCENE_ARGHANION_API UFunctionLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category="Orbit")
	static FVector Eccentricity(FVector R, FVector V, float MU);

	UFUNCTION(BlueprintPure, Category="Orbit")
	static FVector FocusPoint2(float A, FVector E);

	UFUNCTION(BlueprintPure, Category="Orbit")
	static float SemiMajorAxis(FVector R, FVector V, float MU);

	UFUNCTION(BlueprintPure, Category="Orbit")
	static float SemiMinorAxis(float A, float ESquared);

	UFUNCTION(BlueprintPure, Category="Orbit")
	static float PeriodEllipse(float A, float MU);

	UFUNCTION(BlueprintPure, Category="Orbit")
	static float Velocity(float R, float A, float MU);

	UFUNCTION(BlueprintPure, Category="Orbit")
	static float Perimeter(float A, float B);
};
