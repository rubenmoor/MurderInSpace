#pragma once

#include "CoreMinimal.h"
#include "OrbitComponent.h"
#include "GameFramework/Pawn.h"
#include "MyPawn.generated.h"

UCLASS()
class MURDERINSPACE_API AMyPawn : public APawn, public IHasOrbit
{
    GENERATED_BODY()

public:
    // Sets default values for this pawn's properties
    AMyPawn();

    UFUNCTION(BlueprintCallable)
    void UpdateLookTarget(FVector Target);

    // Acceleration in m / s^2
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AccelerationSI = .1;

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
    float RP_bIsAccelerating = false;

    // IOrbit interface
    virtual UOrbitComponent* GetOrbit() override { return Orbit; }
    virtual USceneComponent* GetMovableRoot() override { return MovableRoot; }
    virtual USceneComponent* GetSplineMeshParent() override { return SplineMeshParent; }

    UPROPERTY(ReplicatedUsing=OnRep_BodyRotation, VisibleAnywhere, BlueprintReadOnly)
    FQuat RP_BodyRotation;

    UFUNCTION()
    void OnRep_BodyRotation() { MovableRoot->SetWorldRotation(RP_BodyRotation); }
protected:
    
    // event handlers
    
    virtual void Tick(float DeltaSeconds) override;
    virtual void OnConstruction(const FTransform& Transform) override;
    
    // components
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USceneComponent> Root;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UOrbitComponent> Orbit;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USceneComponent> SplineMeshParent;

    // the root component is stationary, it holds the Orbit component;
    // this scene component holds everything movable
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USceneComponent> MovableRoot;

};