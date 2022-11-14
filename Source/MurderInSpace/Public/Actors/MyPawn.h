#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MyPawn.generated.h"

UCLASS()
class MURDERINSPACE_API AMyPawn : public APawn
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
    virtual UOrbitComponent* GetOrbit()            override final { return Orbit;            }
    virtual USceneComponent* GetMovableRoot()      override final { return MovableRoot;      }
    virtual USceneComponent* GetSplineMeshParent() override final { return SplineMeshParent; }

    UPROPERTY(ReplicatedUsing=OnRep_BodyRotation, VisibleAnywhere, BlueprintReadOnly)
    FQuat RP_BodyRotation;

    UFUNCTION()
    void OnRep_BodyRotation() { GetMovableRoot()->SetWorldRotation(RP_BodyRotation); }
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USceneComponent> MovableRoot;
};