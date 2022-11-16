// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Orbit.h"
#include "MyPlayerStart.generated.h"

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API AMyPlayerStart final : public AActor, public IHasOrbit
{
	GENERATED_BODY()

public:
	virtual AOrbit*             GetOrbit()       override { return Orbit;      }
	virtual void                SetOrbit(AOrbit* InOrbit) override { Orbit = InOrbit; }
	virtual TSubclassOf<AOrbit> GetOrbitClass()  override { return OrbitClass; }

	UFUNCTION(CallInEditor, Category="Orbit")
	void InitializeOrbit();
	
protected:
	// event handlers
	virtual void Destroyed() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	
	// members
	
	UPROPERTY(EditDefaultsOnly, Category="Orbit")
	TSubclassOf<AOrbit> OrbitClass;

	UPROPERTY(EditInstanceOnly, Category="Orbit")
	AOrbit* Orbit;
};
