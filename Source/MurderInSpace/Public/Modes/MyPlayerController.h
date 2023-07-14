#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Orbit/Orbit.h"
#include "GameFramework/PlayerController.h"
#include "Input/MyEnhancedInputComponent.h"
#include "MyPlayerController.generated.h"

struct FGameplayTag;
class UTaggedInputActionData;
/**
 * 
 */
UENUM()
enum class EInputAction : uint8
{
	// given your current orientation, use the main rocket engine to accelerate
	  AccelerateBeginEnd      UMETA(DisplayName = "accelerate")
	, TowardsCircleBeginEnd   UMETA(DisplayName = "accelerate towards circular orbit")
	, EmbraceBeginEnd         UMETA(DisplayName = "use arms to embrace a thing, e.g. an asteroid")
	, KickPositionExecute     UMETA(DisplayName = "use legs to kick something away")
	, KickCancel              UMETA(DisplayName = "cancel the kick")

	// pure UI actions
	, IngameMenuToggle        UMETA(DisplayName = "toggle in-game menu")
	, MyTrajectoryShowHide    UMETA(DisplayName = "show my trajectory")
	, AllTrajectoriesShowHide UMETA(DisplayName = "show all trajectories")
	, MyTrajectoryToggle      UMETA(DisplayName = "toggle my trajectories visibility")
	, Zoom                    UMETA(DisplayName = "zoom the camera in or out")
	, Select                  UMETA(DisplayName = "deselect any selected body")
	
	, MinPureUI = IngameMenuToggle
	, Last = Select
};

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
	
	// change the zoom level continuously
	void Zoom(float Delta);
	
	UFUNCTION(Server, Reliable)
	void ServerRPC_RotateTowards(FQuat Quat);
	
private:
	template<EInputAction InputAction>
	void BindAction()
	{
		const auto* IA = GetInputAction(InputAction);
		if (IsValid(IA))
		{
			Cast<UMyEnhancedInputComponent>(InputComponent)->BindAction
				( IA
				, ETriggerEvent::Triggered
				, this
				, &AMyPlayerController::HandleInputAction<InputAction>
				);
		}
		else
		{
			UE_LOG
				( LogMyGame
				, Error
				, TEXT("%s: couldn't find input action asset for %s")
				, *GetFullName()
				, *UEnum::GetValueAsString(InputAction)
				)
		}
	}

	// for any input action, call 'LocallyHandleAction' and make the RPC only if necessary
	template<EInputAction InputAction>
	void HandleInputAction()
	{
		// 'MinPureUI' marks the enumerator where UI-only actions begin
		// those have only local execution
		// TODO: ? if(InputAction < EInputAction::MinPureUI || GetLocalRole() == ROLE_Authority)
		if(InputAction < EInputAction::MinPureUI)
		{
			ServerRPC_HandleAction(InputAction);
		}
		
		// input action prediction tbd. here
		// TODO: ? if(GetLocalRole() == ROLE_AutonomousProxy)
		
		LocallyHandleAction(InputAction);
	}

	// for gameplay input actions, execute their effects
	UFUNCTION(Server, Reliable)
	void ServerRPC_HandleAction(EInputAction Action);

	// for gameplay input actions AND mere UI interactions: execute their local effects
	void LocallyHandleAction(EInputAction Action);

	// private members

	// input mapping context: in-game
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UInputMappingContext> IMC_InGame;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTaggedInputActionData> MyInputActionsData;

	UPROPERTY(VisibleAnywhere)
    TObjectPtr<UEnhancedInputLocalPlayerSubsystem> Input;

	// input action values for input actions that don't have an InputAction asset
	// e.g. input actions that are triggered by mouse movement
	TMap<EInputAction, FInputActionValue> CustomInputActionValues;

	// private methods
	
	UFUNCTION(BlueprintCallable)
	FVector GetMouseDirection();
	
	// get the value of the InputAction asset given the `EInputAction`
	UFUNCTION(BlueprintCallable)
	FInputActionValue GetInputActionValue(EInputAction InputAction);
	
	// get the InputAction asset given the `EInputAction`
	UFUNCTION(BlueprintCallable)
	UInputAction* GetInputAction(EInputAction InputAction);
	
};
