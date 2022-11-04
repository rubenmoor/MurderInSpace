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

	// Acceleration in m / s^2
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AccelerationSI = .1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float bIsAccelerating = false;

	UFUNCTION(BlueprintCallable)
	UOrbitComponent* GetOrbitComponent() { return Orbit; }

	UFUNCTION(BlueprintCallable)
	USceneComponent* GetBody() { return MovableRoot; }

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	FQuat BodyRotation;
	
protected:
	
	// event handlers
	
	virtual void Tick(float DeltaSeconds) override;

	#if WITH_EDITOR
	/*
	 * the `OnConstruction` script gets called when running the game ... unfortunately, the component `MovableRoot`
	 * has weird location values (e.g. 1.0E-312) and as the final values for `Physics` are available only with
	 * `BeginPlay`, we don't need to run `OnConstruction`, and rely on `BeginPlay` to set up the orbit.
	 */
	virtual void OnConstruction(const FTransform& Transform) override;
	#endif
	
	virtual void BeginPlay() override;

	// components
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UOrbitComponent> Orbit;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<USceneComponent> SplineMeshParent;

	// the root component is stationary, it holds the Orbit component;
	// this scene component holds everything movable
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<USceneComponent> MovableRoot;

};
