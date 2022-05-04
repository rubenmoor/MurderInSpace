// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Orbit.h"
#include "GameFramework/Pawn.h"
#include "PawnInSpace.generated.h"

UCLASS()
class SPACESCENE_ARGHANION_API APawnInSpace : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APawnInSpace();

	UFUNCTION(BlueprintCallable)
	void UpdateMU(float MU, float RMAX) const;

	UFUNCTION(BlueprintCallable)
	void UpdateLookTarget(FVector Target);

	UFUNCTION(BlueprintCallable)
	virtual void LookAt(FVector VecP);
	
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
