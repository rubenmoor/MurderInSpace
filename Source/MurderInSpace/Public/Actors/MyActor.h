// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OrbitComponent.h"
#include "MyActor.generated.h"

class UOrbitComponent;

UCLASS()
class MURDERINSPACE_API AMyActor : public AActor, public IHasOrbit
{
    GENERATED_BODY()

public:	
    // Sets default values for this actor's properties
    AMyActor();

    // IOrbit interface
    virtual UOrbitComponent* GetOrbit() override final { return Orbit; }
    virtual USceneComponent* GetMovableRoot() override final { return MovableRoot; }
    virtual USceneComponent* GetSplineMeshParent() override final { return SplineMeshParent; }
    
protected:
    // event handlers

    virtual void OnConstruction(const FTransform& Transform) override;

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

