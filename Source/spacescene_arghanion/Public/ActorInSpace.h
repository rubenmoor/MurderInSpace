// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Orbit.h"
#include "GameFramework/Actor.h"
#include "ActorInSpace.generated.h"

UCLASS()
class SPACESCENE_ARGHANION_API AActorInSpace : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	AActorInSpace();

	UFUNCTION(BlueprintCallable)
	void UpdateMU(float MU, float RMAX) const;

protected:
	// event handlers
	
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	// components
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> Root;

	// member variables

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AOrbit> Orbit;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UClass> OrbitClass;
	
	// private methods

	UFUNCTION(BlueprintCallable)
	void SpawnOrbit();
};

