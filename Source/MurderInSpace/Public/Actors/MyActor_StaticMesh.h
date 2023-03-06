// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IHasMesh.h"
#include "Orbit.h"
#include "MyComponents/MyCollisionComponent.h"
#include "MyActor_StaticMesh.generated.h"

class UGeometryCollectionComponent;
class UGyrationComponent;
/**
 * 
 */
UCLASS()
class MURDERINSPACE_API AMyActor_StaticMesh
	: public AActor
	, public IHasMesh
	, public IHasOrbit
	, public IHasOrbitColor
	, public IHasCollision
{
	GENERATED_BODY()

public:
	AMyActor_StaticMesh();

	virtual UPrimitiveComponent* GetMesh()   const override { return StaticMesh; }
	virtual float                GetMyMass() const override { return MyMassOverride == 0. ? MyMass : MyMassOverride; }
	virtual TSubclassOf<AOrbit>  GetOrbitClass()   override { return OrbitClass;   }
	virtual FLinearColor         GetOrbitColor()   override { return OrbitColor;   }
	virtual AOrbit*				 GetOrbit() const  override { return RP_Orbit;     }
	virtual void 				 SetOrbit(AOrbit* InOrbit) override { RP_Orbit = InOrbit; }
	virtual UMyCollisionComponent* GetCollisionComponent() override { return Collision; }
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
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

	// components

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> StaticMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UGyrationComponent> Gyration;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UMyCollisionComponent> Collision;

	// members
	
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category="Orbit")
	AOrbit* RP_Orbit = nullptr;
	
	UPROPERTY(EditDefaultsOnly, Category="Orbit")
	TSubclassOf<AOrbit> OrbitClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Orbit")
    FLinearColor OrbitColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Orbit")
	float MyMass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Orbit")
	float MyMassOverride = 0.;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Orbit")
	FInitialOrbitParams InitialOrbitParams;
	
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bInitialOrbitParamsSet = false;
    
};
