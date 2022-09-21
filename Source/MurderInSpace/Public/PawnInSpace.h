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

	// event handlers
	virtual void Tick(float DeltaSeconds) override;

protected:
	
	// components
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UOrbitComponent> Orbit;
};
