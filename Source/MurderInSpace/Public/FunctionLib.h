// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModeAsteroids.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FunctionLib.generated.h"

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UFunctionLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Orbit")
	static FVector Eccentricity(FVector R, FVector V, float Alpha);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Orbit")
	static FVector FocusPoint2(float A, FVector E);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Orbit")
	static float SemiMajorAxis(FVector VecR, FVector VecV, float Alpha);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Orbit")
	static float SemiMinorAxis(float A, float ESquared);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Orbit")
	static float PeriodEllipse(float A, float Alpha);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Orbit")
	static float Perimeter(float A, float B);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Orbit")
	static float AxialTidalForce(float R, float L, float M, float Alpha);

	static TObjectPtr<AGameModeAsteroids> GetGameModeAsteroids(const UObject* WorldContextObject);

	void LogVector(FLogCategoryName CategoryName, ELogVerbosity::Type Verbosity, FVector Vector);
};
