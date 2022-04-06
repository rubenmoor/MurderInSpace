// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "ABlackhole.generated.h"

UCLASS()
class SPACESCENE_ARGHANION_API AABlackhole : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AABlackhole();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly);
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Required")
	UStaticMeshComponent* EventHorizon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Required")
	UStaticMeshComponent* GravitationalLens;
};
