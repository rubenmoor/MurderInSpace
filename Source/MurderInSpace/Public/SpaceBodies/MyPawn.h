#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayCueInterface.h"
#include "Orbit/Orbit.h"
#include "GameFramework/Pawn.h"

#include "MyPawn.generated.h"

class UGameplayAbility;
class UMyGameplayAbility;
class UAttrSetAcceleration;
class UMyAbilitySystemComponent;

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
	, public IAbilitySystemInterface
	, public IGameplayCueInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMyPawn();
	
	// IOrbit interface
	virtual TSubclassOf<AOrbit> GetOrbitClass()  override { return OrbitClass; }
	virtual FLinearColor        GetOrbitColor()  override { return OrbitColor; }
	virtual AOrbit*				GetOrbit() const override { return RP_Orbit; };
	virtual void 				SetOrbit(AOrbit* InOrbit) override { RP_Orbit = InOrbit; };
	virtual FInitialOrbitParams GetInitialOrbitParams() const override { return InitialOrbitParams; }
	virtual void SetInitialOrbitParams(const FInitialOrbitParams& InParams) override { InitialOrbitParams = InParams; }

	// ability system interface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return static_cast<UAbilitySystemComponent*>(AbilitySystemComponent); }

	// get current angular velocity
	UFUNCTION(BlueprintCallable)
	double GetOmega() const { return Omega; }

	// "cheat" and allow to set Omega to an arbitrary value for more accuracy in the LookAt ability
	UFUNCTION(BlueprintCallable)
	void SetOmega(float InOmega);
	
protected:
    // event handlers
    
    virtual void Tick(float DeltaSeconds) override;
    virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	
    // server-only
    virtual void PossessedBy(AController* NewController) override;
    
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

    // gameplay ability system
	
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UMyAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttrSetAcceleration> AttrSetAcceleration;

	// angular velocity in radians per second
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	double Omega = 0.;

	// private methods

	UFUNCTION()
	void OnRep_Orbit();

	void SetReadyFlags(EMyPawnReady ReadyFlags);
	void Initialize();
};