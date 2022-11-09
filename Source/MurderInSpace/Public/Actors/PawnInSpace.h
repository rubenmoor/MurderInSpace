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

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
	float RP_bIsAccelerating = false;

	UFUNCTION(BlueprintCallable)
	UOrbitComponent* GetOrbitComponent() const { return Orbit; }

	UFUNCTION(BlueprintCallable)
	USceneComponent* GetBody() { return MovableRoot; }

	UPROPERTY(ReplicatedUsing=OnRep_BodyRotation, VisibleAnywhere, BlueprintReadOnly)
	FQuat RP_BodyRotation;

	UFUNCTION()
	void OnRep_BodyRotation() { MovableRoot->SetWorldRotation(RP_BodyRotation); }
protected:
	
	// event handlers
	
	virtual void Tick(float DeltaSeconds) override;

	#if WITH_EDITOR
	virtual void OnConstruction(const FTransform& Transform) override;
	#endif
	
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
