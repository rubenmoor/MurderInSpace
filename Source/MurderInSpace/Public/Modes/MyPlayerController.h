#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputSubsystems.h"
#include "GameplayAbilitySpec.h"
#include "InputMappingContext.h"
#include "Orbit/Orbit.h"
#include "GameFramework/PlayerController.h"

#include "MyPlayerController.generated.h"

class UMyGameplayAbility;
class UEnhancedInputComponent;
struct FGameplayTagContainer;
class UMyInputActionSet;
class UMyInputActions;
class UMyAbilitySystemComponent;
struct FGameplayTag;
class UTaggedInputActionData;

UENUM(BlueprintType)
enum class EInputTrigger : uint8
{
      Down			 UMETA(DisplayName="Down"         )
    , Pressed		 UMETA(DisplayName="Pressed"      )
    , Released		 UMETA(DisplayName="Released"     )
    , Hold		     UMETA(DisplayName="Hold"         )
    , HoldAndRelease UMETA(DisplayName="HoldAndRelase")
    , Tap		     UMETA(DisplayName="Tap"          )
    , Pulse		     UMETA(DisplayName="Pulse"        )
    , ChordAction	 UMETA(DisplayName="ChordAction"  )
};

USTRUCT(BlueprintType)
struct FHighlight
{
	GENERATED_BODY()

	FHighlight(): Orbit(nullptr), Size(0) {}
	FHighlight(class AOrbit* InOrbit, double InSize) : Orbit(InOrbit), Size(InSize) {}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AOrbit* Orbit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double Size;
};

USTRUCT(BlueprintType)
struct FDuplicateKeyBinding
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FKey Key;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UMyInputActionSet> IAS1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UMyInputActionSet> IAS2;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TSet<EInputTrigger> SetTriggers;
};

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

	AMyPlayerController();

	// setup defaults for my input actions:
	// * bind them to gameplay abilities/cues by tag
	// * or give special treatment
	virtual void SetupInputComponent() override;

public:
	UFUNCTION(BlueprintCallable)
	void CheckForDuplicateKeyBindings(UMyInputActionSet* IAS1, TArray<FDuplicateKeyBinding>& DuplicateKeyBindings);
	
	UFUNCTION(Client, Reliable)
	void ClientRPC_LeaveSession();

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<class AMyPlayerStart> MyPlayerStart;
	
    DECLARE_MULTICAST_DELEGATE_OneParam(FShowAllOrbitsDelegate, bool)

	/*
	 * broadcast to change orbit visibility for all orbits
	 * bool bShowHide: true = show, false = hide
	 */
	FShowAllOrbitsDelegate ShowAllOrbitsDelegate;

	UFUNCTION(BlueprintCallable)
	FHighlight GetHovered() const { return Hovered; }
	
	UFUNCTION(BlueprintCallable)
	FHighlight GetSelected() const { return Selected; }
	
	UFUNCTION(BlueprintCallable)
	UEnhancedInputComponent* GetInputComponent();

	UFUNCTION(BlueprintCallable)
	void RunInputAction(const FGameplayTagContainer& InputActionTags, EInputTrigger InputTrigger, const FInputActionInstance& InputActionInstance);
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 CameraPosition = 2;

	static constexpr uint8 MaxCameraPosition = 7;

	// event handlers
	
	virtual void Tick(float DeltaSeconds) override;
	
	// server-only
	virtual void OnPossess(APawn* InPawn) override;
	
	// OnPossess only runs on the server (in a listen-server setup)
	//virtual void OnPossess(APawn* InPawn) override;
	// Thus we use `AcknowledgePossesion` to set up the camera and alike
	virtual void AcknowledgePossession(APawn* P) override;

	virtual void BeginPlay() override;

	// input events

	/*
	 * deal with input actions bindings apart from gameplay ability or cue
	 * (an input action can have any combination of bindings to custom, ability, cue
	*/
	//UFUNCTION() TODO
	void RunCustomInputAction(FGameplayTag CustomBindingTag, EInputTrigger InputTrigger, const FInputActionInstance& InputActionInstance);
	
	// state for tap input action
	UPROPERTY(VisibleAnywhere)
	bool bMyOrbitShowHide = false;
	
	// for gameplay input actions AND mere UI interactions: execute their local effects
	//void LocallyHandleAction(EInputAction Action, const FInputActionInstance& IAInstance);

	// private members

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UMyInputActions> MyInputActions;

	UPROPERTY(VisibleAnywhere)
    TObjectPtr<UEnhancedInputLocalPlayerSubsystem> Input;

	// TODO probably delete, no need to use my action enum anymore
	// input action values for input actions that don't have an InputAction asset
	// e.g. input actions that are triggered by mouse movement
	//TMap<EInputAction, FInputActionValue> CustomInputActionValues;

	// private methods
	
	UFUNCTION(BlueprintCallable)
	FVector GetMouseDirection();

	// convenience access to the AbilitySystemComponent of the possessed pawn
	// only used by client to process input
	UPROPERTY(BlueprintReadOnly)
	UMyAbilitySystemComponent* AbilitySystemComponent;

	// the selected body and its orbit
	UPROPERTY()
	FHighlight Selected;
	
	// the hovered body and its orbit, based on mouse movement
	UPROPERTY()
	FHighlight Hovered;

	// the rotation angle in radians between UnitX and the vector pointing from screen center to mouse position
	UPROPERTY()
	float MouseAngle = 0.;

	// debugging
	FVector VecAngle = FVector::Zero();

	FDelegateHandle DelegateHandleOnLookAt;
};
