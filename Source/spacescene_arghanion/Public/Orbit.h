// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <variant>

#include "CoreMinimal.h"
#include "Orbit.generated.h"

UCLASS()
class Orbit

USTRUCT(BlueprintType)
struct FCircle
{
    GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float R;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector VecH;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float T;
};

USTRUCT(BlueprintType)
struct FEllipse
{
    GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float A;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector VecH;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector VecE;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float T;
};

USTRUCT(BlueprintType)
struct FParabola
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float P;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector VecE;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector VecH;
};

USTRUCT(BlueprintType)
struct FHyperbola
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float A;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float B;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector VecE;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector VecH;
};

//using Orbit = std::variant<FCircle, FEllipse, FParabola, FHyperbola>;