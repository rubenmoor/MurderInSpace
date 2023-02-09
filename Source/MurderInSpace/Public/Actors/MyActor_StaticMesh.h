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
class MURDERINSPACE_API AMyActor_StaticMesh final
	: public AActor
	, public IHasMesh
	, public IHasOrbit
	, public IHasOrbitColor
	, public IHasCollision
{
	GENERATED_BODY()

	AMyActor_StaticMesh();

public:
	virtual UPrimitiveComponent* GetMesh()   const override { return StaticMesh; }
	virtual float                GetMyMass() const override { return MyMassOverride == 0. ? MyMass : MyMassOverride; }
	virtual TSubclassOf<AOrbit>  GetOrbitClass()   override { return OrbitClass;   }
	virtual FLinearColor         GetOrbitColor()   override { return OrbitColor;   }
	virtual AOrbit*				 GetOrbit() const  override { return RP_Orbit;     }
	virtual void 				 SetOrbit(AOrbit* InOrbit) override { RP_Orbit = InOrbit; }
	virtual UMyCollisionComponent* GetCollisionComponent() override { return Collision; }
	
protected:
	// event handlers
	//virtual void BeginPlay() override;
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
	TObjectPtr<UGeometryCollectionComponent> GeometryCollection;
	
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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bGeometryCollectionSetupDone = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Orbit")
	float MyMass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Orbit")
	float MyMassOverride = 0.;
};
