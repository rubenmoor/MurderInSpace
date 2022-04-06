// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Orbit.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "AObject.generated.h"

UCLASS()
class SPACESCENE_ARGHANION_API AAObject : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAObject();

	virtual void Tick(float DeltaTime) override;

protected:

	// member variables

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SplineDistance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString OrbitInformation;

	// UE does not support variant
	//UPROPERTY()
	Orbit MyOrbit;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float VelocityScalar;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector Velocity;

	// components
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USplineComponent* Spline;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* MeshRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* MainMesh;
	
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
};

