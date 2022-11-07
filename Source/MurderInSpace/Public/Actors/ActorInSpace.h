// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActorInSpace.generated.h"

class UOrbitComponent;
class UGyrationComponent;

UCLASS()
class MURDERINSPACE_API AActorInSpace : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	AActorInSpace();
	
protected:
	// event handlers

	virtual void OnConstruction(const FTransform& Transform) override;
	//virtual void BeginPlay() override;

	// components
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<USceneComponent> Root;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UOrbitComponent> Orbit;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<USceneComponent> SplineMeshParent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<USceneComponent> MovableRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UGyrationComponent> Gyration;

	// input event handlers

	UFUNCTION()
	void HandleBeginMouseOver(UPrimitiveComponent* Body);
	
	UFUNCTION()
	void HandleEndMouseOver(UPrimitiveComponent* Body);
	
	UFUNCTION()
	void HandleClick(UPrimitiveComponent* Body, FKey Button);
};

