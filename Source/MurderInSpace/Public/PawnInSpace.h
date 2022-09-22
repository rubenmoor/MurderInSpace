// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OrbitComponent.h"
#include "GameFramework/Pawn.h"
#include "PawnInSpace.generated.h"

UCLASS()
class MURDERINSPACE_API APawnInSpace : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APawnInSpace();

	UFUNCTION(BlueprintCallable)
	void UpdateLookTarget(FVector Target);

	UFUNCTION(BlueprintCallable)
	virtual void LookAt(FVector VecP);

	UFUNCTION(BlueprintCallable)
	UOrbitComponent* GetOrbitComponent() { return Orbit; }

	// Acceleration in m / s^2
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AccelerationSI = .5;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float bIsAccelerating = false;

protected:
	
	// event handlers
	virtual void Tick(float DeltaSeconds) override;

	virtual void BeginPlay() override;
	// components
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UOrbitComponent> Orbit;
	
	// the root component is stationary, it holds the Orbit component;
	// this scene component holds everything movable
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<USceneComponent> MovableRoot;

};
