#pragma once

#include "CoreMinimal.h"
#include "Orbit/Orbit.h"
#include "GameFramework/Pawn.h"
#include "MyPawn.generated.h"

UENUM(meta=(Bitflags))
enum class EMyPawnReady : uint8
{
	None          = 0,
	InternalReady = 1 << 0,
	OrbitReady    = 1 << 1,

	All = InternalReady | OrbitReady
};
ENUM_CLASS_FLAGS(EMyPawnReady)

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
    double AccelerationSI = 1.0;

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
    bool RP_bIsAccelerating = false;

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
    bool RP_bTowardsCircle = false;

    // IOrbit interface
	virtual TSubclassOf<AOrbit> GetOrbitClass()  override { return OrbitClass; }
	virtual FLinearColor        GetOrbitColor()  override { return OrbitColor; }
	virtual AOrbit*				GetOrbit() const override { return RP_Orbit; };
	virtual void 				SetOrbit(AOrbit* InOrbit) override { RP_Orbit = InOrbit; };
	virtual FInitialOrbitParams GetInitialOrbitParams() const override { return InitialOrbitParams; }
	virtual void SetInitialOrbitParams(const FInitialOrbitParams& InParams) override { InitialOrbitParams = InParams; }
	
    UPROPERTY(ReplicatedUsing=OnRep_Rotation, VisibleAnywhere, BlueprintReadOnly)
    FQuat RP_Rotation;

    UFUNCTION()
    void OnRep_Rotation() { SetActorRotation(RP_Rotation); }
	
protected:
    // event handlers
    
    virtual void Tick(float DeltaSeconds) override;
    virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
    
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
	
	virtual void Destroyed() override;
    
    // components
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USceneComponent> Root;
    
	// members

	UPROPERTY(ReplicatedUsing=OnRep_Orbit, VisibleAnywhere, BlueprintReadOnly, Category="Orbit")
	AOrbit* RP_Orbit = nullptr;
	
	UPROPERTY(EditAnywhere, Category="Orbit")
	TSubclassOf<AOrbit> OrbitClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Orbit")
    FLinearColor OrbitColor;

	EMyPawnReady MyPawnReady = EMyPawnReady::None;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Orbit")
	FInitialOrbitParams InitialOrbitParams;

	// private methods

	UFUNCTION()
	void OnRep_Orbit();

	void SetReadyFlags(EMyPawnReady ReadyFlags);
};