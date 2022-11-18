#pragma once

#include "CoreMinimal.h"
#include "Orbit.h"
#include "GameFramework/Pawn.h"
#include "MyPawn.generated.h"

UCLASS()
class MURDERINSPACE_API AMyPawn
	: public APawn
	, public IHasOrbit
	, public IHasOrbitColor
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
	virtual TSubclassOf<AOrbit> GetOrbitClass()  override { return OrbitClass; }
	virtual FLinearColor        GetOrbitColor()  override { return OrbitColor; }
	
    UPROPERTY(ReplicatedUsing=OnRep_BodyRotation, VisibleAnywhere, BlueprintReadOnly)
    FQuat RP_Rotation;

    UFUNCTION()
    void OnRep_BodyRotation() { Root->SetWorldRotation(RP_Rotation); }
	
protected:
    // event handlers
    
    virtual void Tick(float DeltaSeconds) override;
    virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
	virtual void Destroyed() override;
    
    // components
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USceneComponent> Root;
    
	// members

	UPROPERTY(EditDefaultsOnly, Category="Orbit")
	TSubclassOf<AOrbit> OrbitClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Orbit")
    FLinearColor OrbitColor;
};