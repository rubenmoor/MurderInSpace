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
	virtual TSubclassOf<AOrbit> GetOrbitClass()  override { return OrbitClass; }
	
protected:
	virtual FString GetDefaultActorLabel() const override { return GetClass()->GetName(); }

	// event handlers
	virtual void OnConstruction(const FTransform& Transform) override;
	
	// members
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AOrbit> OrbitClass;
	
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
	AOrbit* Orbit;
};
