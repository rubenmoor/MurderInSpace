#pragma once

#include "CoreMinimal.h"
#include "Orbit/Orbit.h"
#include "MyPlayerStart.generated.h"

class UCapsuleComponent;

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API AMyPlayerStart final : public AActor, public IHasOrbit
{
	GENERATED_BODY()

public:
	AMyPlayerStart();
	
	virtual TSubclassOf<AOrbit> GetOrbitClass()  override { return OrbitClass; }
	virtual AOrbit*				GetOrbit() const override { return Orbit; };
	virtual void 				SetOrbit(AOrbit* InOrbit) override { Orbit = InOrbit; };
	virtual FInitialOrbitParams GetInitialOrbitParams() const override { return InitialOrbitParams; }
	virtual void SetInitialOrbitParams(const FInitialOrbitParams& InParams) override { InitialOrbitParams = InParams; }
	
protected:
	// event handlers
	virtual void Destroyed() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	
	// members

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCapsuleComponent> CollisionCapsule;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Orbit")
	AOrbit* Orbit = nullptr;
	
	UPROPERTY(EditAnywhere, Category="Orbit")
	TSubclassOf<AOrbit> OrbitClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Orbit")
	FInitialOrbitParams InitialOrbitParams;
};
