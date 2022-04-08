// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "KeplerOrbitComponent.h"
#include "GameFramework/Actor.h"
#include "AObjectInSpace.generated.h"

UCLASS()
class SPACESCENE_ARGHANION_API AAObjectInSpace : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAObjectInSpace();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void UpdateMU(float MU) const;

protected:

	// member variables

	// components
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UKeplerOrbitComponent* Orbit;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* MeshRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* MainMesh;
	
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
};

