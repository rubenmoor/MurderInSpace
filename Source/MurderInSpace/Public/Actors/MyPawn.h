#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Orbit/Orbit.h"
#include "GameFramework/Pawn.h"

#include "MyPawn.generated.h"

class UGameplayAbility;
class UMyInputGameplayAbility;
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
{
    GENERATED_BODY()

public:
    // Sets default values for this pawn's properties
    AMyPawn();
	
    UFUNCTION(BlueprintCallable)
    void UpdateLookTarget(FVector Target);

    // IOrbit interface
	virtual TSubclassOf<AOrbit> GetOrbitClass()  override { return OrbitClass; }
	virtual FLinearColor        GetOrbitColor()  override { return OrbitColor; }
	virtual AOrbit*				GetOrbit() const override { return RP_Orbit; };
	virtual void 				SetOrbit(AOrbit* InOrbit) override { RP_Orbit = InOrbit; };
	virtual FInitialOrbitParams GetInitialOrbitParams() const override { return InitialOrbitParams; }
	virtual void SetInitialOrbitParams(const FInitialOrbitParams& InParams) override { InitialOrbitParams = InParams; }
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return static_cast<UAbilitySystemComponent*>(AbilitySystemComponent); }

	void SetRotationAim(const FQuat& InQuat);

protected:
    // event handlers
    
    virtual void Tick(float DeltaSeconds) override;
    virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	
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

	// this abilities will be given to the pawn in BeginPlay
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<UGameplayAbility>> Abilities;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAttrSetAcceleration> AttrSetAcceleration;

	// angular velocity in radians per second
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	double Omega = 0.;

	// the rotation towards which the pawn is currently rotating
    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
    FQuat RP_QuatRotationAim = FQuat::Identity;

	// private methods

	UFUNCTION()
	void OnRep_Orbit();

	void SetReadyFlags(EMyPawnReady ReadyFlags);
	void Initialize();
};