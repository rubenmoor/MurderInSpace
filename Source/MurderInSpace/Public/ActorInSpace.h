// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Orbit.h"
#include "GameFramework/Actor.h"
#include "ActorInSpace.generated.h"

UCLASS()
class MURDERINSPACE_API AActorInSpace : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	AActorInSpace();

protected:
	// event handlers
	
	// components
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> Root;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UOrbitDataComponent> OrbitData;
};

