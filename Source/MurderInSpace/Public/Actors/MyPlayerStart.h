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
	AMyPlayerStart();
	
	virtual TSubclassOf<AOrbit> GetOrbitClass()  override { return OrbitClass; }
	virtual AOrbit*				GetOrbit() const override { return RP_Orbit; };
	virtual void 				SetOrbit(AOrbit* InOrbit) override { RP_Orbit = InOrbit; };
    virtual void SetInitialOrbitParams(FInitialOrbitParams InParams) override
	{
		InitialOrbitParams = InParams;
		bInitialOrbitParamsSet = true;	
	}
	virtual FInitialOrbitParams GetInitialOrbitParams() const override { return InitialOrbitParams; }
	virtual bool GetBInitialOrbitParamsSet() override { return bInitialOrbitParamsSet; }
	
protected:
	// event handlers
	virtual void Destroyed() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	
	// members
	
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category="Orbit")
	AOrbit* RP_Orbit = nullptr;
	
	UPROPERTY(EditDefaultsOnly, Category="Orbit")
	TSubclassOf<AOrbit> OrbitClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Orbit")
	FInitialOrbitParams InitialOrbitParams;
	
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Orbit")
    bool bInitialOrbitParamsSet = false;
};
