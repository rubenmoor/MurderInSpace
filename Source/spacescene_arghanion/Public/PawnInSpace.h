// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "KeplerOrbitComponent.h"
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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> VisualTrajectory;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UKeplerOrbitComponent> Orbit;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> MeshRoot;

	virtual void BeginPlay() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
};
