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

	void SetRotationAim(const FQuat& Quat);

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

	// angular velocity in radians per second
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	double Omega = 0.;

	// maximum angular velocity
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	double RP_OmegaMax = 60. * PI / 180.;

	// angular acceleration in radians per second squared
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	double Alpha = 0.;

	// angular acceleration in radians per second squared
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	double RP_AlphaMax = 60. * PI / 180.;

	// the rotation towards which the pawn is currently rotating
    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
    FQuat RP_RotationAim = FQuat::Identity;

	// private methods

	UFUNCTION()
	void OnRep_Orbit();

	void SetReadyFlags(EMyPawnReady ReadyFlags);

	// debugging
	int32 NTicks = 0;
};