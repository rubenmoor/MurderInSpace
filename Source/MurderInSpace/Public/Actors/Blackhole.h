// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blackhole.generated.h"

UCLASS()
class MURDERINSPACE_API ABlackhole : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABlackhole();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly);
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Required")
	TObjectPtr<UStaticMeshComponent> EventHorizon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Required")
	TObjectPtr<UStaticMeshComponent> GravitationalLens;
};
