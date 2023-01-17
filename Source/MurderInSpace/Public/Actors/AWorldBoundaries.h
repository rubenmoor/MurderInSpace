// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "AWorldBoundaries.generated.h"

UCLASS()
class MURDERINSPACE_API AAWorldBoundaries : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAWorldBoundaries();

protected:
	// Called when the game starts or when spawned
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	TObjectPtr<USphereComponent> Sphere;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};