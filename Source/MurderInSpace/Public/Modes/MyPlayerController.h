#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Orbit/Orbit.h"
#include "GameFramework/PlayerController.h"
#include "Input/MyEnhancedInputComponent.h"
#include "MyPlayerController.generated.h"

class UMyAbilitySystemComponent;
struct FGameplayTag;
class UTaggedInputActionData;

// / //**
//  * 
//  */
// UENUM()
// enum class EInputAction : uint8
// {
// 	// given your current orientation, use the main rocket engine to accelerate
// 	  AccelerateBeginEnd      UMETA(DisplayName = "accelerate")
// 	, TowardsCircleBeginEnd   UMETA(DisplayName = "accelerate towards circular orbit")
// 	, EmbraceBeginEnd         UMETA(DisplayName = "use arms to embrace a thing, e.g. an asteroid")
// 	, KickPositionExecute     UMETA(DisplayName = "use legs to kick something away")
// 	, KickCancel              UMETA(DisplayName = "cancel the kick")
// 
// 	// pure UI actions
// 	, IngameMenuToggle        UMETA(DisplayName = "toggle in-game menu")
// 	, MyTrajectoryShowHide    UMETA(DisplayName = "show my trajectory")
// 	, AllTrajectoriesShowHide UMETA(DisplayName = "show all trajectories")
// 	, MyTrajectoryToggle      UMETA(DisplayName = "toggle my trajectories visibility")
// 	, Zoom                    UMETA(DisplayName = "zoom the camera in or out")
// 	, Select                  UMETA(DisplayName = "deselect any selected body")
// 	
// 	, MinPureUI = IngameMenuToggle
// 	, Last = Select
// };

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

	AMyPlayerController();
	virtual void SetupInputComponent() override;

public:
	UFUNCTION(Client, Reliable)
	void ClientRPC_LeaveSession();

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<class AMyPlayerStart> MyPlayerStart;
	
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
	UFUNCTION()
	void HandlePressed(const FInputActionInstance& InputActionInstance);
	
	UFUNCTION()
	void HandleReleased(const FInputActionInstance& InputActionInstance);
	
	UFUNCTION()
	void HandleTapped(const FInputActionInstance& InputActionInstance);
	
	
	// change the zoom level continuously
	void Zoom(float Delta);
	
	UFUNCTION(Server, Reliable)
	void ServerRPC_RotateTowards(FQuat Quat);
	
	// for gameplay input actions AND mere UI interactions: execute their local effects
	//void LocallyHandleAction(EInputAction Action, const FInputActionInstance& IAInstance);

	// private members

	// input mapping context: in-game
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UInputMappingContext> IMC_InGame;

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

	UFUNCTION(BlueprintCallable)
	UMyEnhancedInputComponent* GetInputComponent() { return Cast<UMyEnhancedInputComponent>(InputComponent); }
};
