// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IHasMesh.h"
#include "MathUtil.h"
#include "RealtimeMeshActor.h"
#include "Orbit/Orbit.h"
#include "MyComponents/MyCollisionComponent.h"

#include "MyActor_RealtimeMesh.generated.h"

class UGyrationComponent;

/*
 *
 */
UCLASS()
class MURDERINSPACE_API AMyActor_RealtimeMesh
    : public ARealtimeMeshActor
	, public IHasMesh
	, public IHasOrbit
	, public IHasOrbitColor
	, public IHasCollision
{
    GENERATED_BODY()

public:
    AMyActor_RealtimeMesh();

	// IHasMesh
	virtual TArray<UPrimitiveComponent*> GetPrimitiveComponents() const override { return {RealtimeMeshComponent}; }
	virtual FBoxSphereBounds     GetBounds() const override { return RealtimeMeshComponent->Bounds; }
	virtual double               GetMyMass() const override { return MyMassOverride == 0. ? MyMass : MyMassOverride; }
	virtual FVector              GetMyInertiaTensor() const override { return RotInertiaNorm; }

	// IHasOrbit
	virtual TSubclassOf<AOrbit>  GetOrbitClass()   override { return OrbitClass;   }
	virtual AOrbit*				 GetOrbit() const  override { return RP_Orbit;     }
	virtual void 				 SetOrbit(AOrbit* InOrbit) override { RP_Orbit = InOrbit; }
	virtual FInitialOrbitParams  GetInitialOrbitParams() const override { return InitialOrbitParams; }
	virtual void                 SetInitialOrbitParams(const FInitialOrbitParams& InParams) override { InitialOrbitParams = InParams; }

	// IHasOrbitColor
	virtual FLinearColor         GetOrbitColor()   override { return OrbitColor;   }
	
	// IHasCollision
	virtual UMyCollisionComponent* GetCollisionComponent() override { return CollisionComponent; }
	
protected:
	// event handler overrides
	virtual void Destroyed() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

	// components

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UGyrationComponent> Gyration;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UMyCollisionComponent> CollisionComponent;

	// members
	
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category="Orbit")
	AOrbit* RP_Orbit = nullptr;
	
	UPROPERTY(EditAnywhere, Category="Orbit")
	TSubclassOf<AOrbit> OrbitClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Orbit")
    FLinearColor OrbitColor;

	// volume of the mesh, used for calculation of my mass
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Orbit")
	double MyVolume;

	// custom mass calculation, using volume (from mesh generation) and density (from physical material)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Orbit")
	double MyMass;

	// override the calculation of my mass (via volume and density) with a set value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Orbit")
	double MyMassOverride = 0.;

	// principal moments of inertia, normalized
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Orbit")
	FVector RotInertiaNorm = FVector(FMathd::InvSqrt3, FMathd::InvSqrt3, FMathd::InvSqrt3);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Orbit")
	FInitialOrbitParams InitialOrbitParams;

	// destruction animation

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Damage")
	TSubclassOf<AActor> DebrisClass;

	// damage

	static double constexpr StructurePointsMaxInitial = 100.;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage")
	double StructurePointsMax = StructurePointsMaxInitial;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage")
	double StructurePointsCurrent = StructurePointsMaxInitial;
};
