// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OrbitComponent.h"

/**
 * 
 */
template<class C>
class MURDERINSPACE_API WithOrbit : public C, public IHasOrbit
{
    //static_assert(std::is_base_of_v<AActor, C>, "C must be a descendant of SomeClass");
public:
	WithOrbit()
	{
    AActor* Me = Cast<AActor>(this);
    Me->PrimaryActorTick.bCanEverTick = true;

    // components

    Root = Me->CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    Root->SetMobility(EComponentMobility::Stationary);
    Me->SetRootComponent(Root);

    MovableRoot = Me->CreateDefaultSubobject<USceneComponent>(TEXT("MovableRoot"));
    MovableRoot->SetupAttachment(Root);
    
    Orbit = Me->CreateDefaultSubobject<UOrbitComponent>(TEXT("Orbit"));
    Orbit->SetupAttachment(Root);

    SplineMeshParent = Me->CreateDefaultSubobject<USceneComponent>(TEXT("SplineMesh"));
    SplineMeshParent->SetupAttachment(Orbit);
    SplineMeshParent->SetMobility(EComponentMobility::Stationary);
    
    Me->bNetLoadOnClient = false;
    //Me->bReplicates = true;
    Me->SetReplicates(true);
    Me->SetReplicateMovement(false);
	}

    // IOrbit interface
    virtual UOrbitComponent* GetOrbit() override final { return Orbit; }
    virtual USceneComponent* GetMovableRoot() override final { return MovableRoot; }
    virtual USceneComponent* GetSplineMeshParent() override final { return SplineMeshParent; }
    
protected:
    // event handlers
    
    virtual void OnConstruction(const FTransform& Transform) override
    {
        //Super::OnConstruction(Transform);
        IHasOrbit::Construction(Transform);
    }
    
    // components
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USceneComponent> Root;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UOrbitComponent> Orbit;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USceneComponent> SplineMeshParent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USceneComponent> MovableRoot;
};

class MURDERINSPACE_API MyNewActor : public WithOrbit<AActor>
{
    
};

class MURDERINSPACE_API MyNewPawn : public WithOrbit<APawn>
{
    
};