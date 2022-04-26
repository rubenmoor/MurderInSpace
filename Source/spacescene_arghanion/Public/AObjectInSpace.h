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

	UFUNCTION(BlueprintCallable)
	void UpdateMU(float MU, float RMAX) const;

protected:

	// member variables

	// components
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UKeplerOrbitComponent> Orbit;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> MeshRoot;

	virtual void BeginPlay() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
};

