#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Orbit/Orbit.h"
#include "GameFramework/PlayerController.h"
#include "Input/MyEnhancedInputComponent.h"
#include "MyPlayerController.generated.h"

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
	void ServerRPC_LookAt(FQuat Quat);
	
private:
	template<EInputAction InputAction>
	void BindAction()
	{
		auto* MyState = UMyState::Get();
		const auto* IA = MyState->GetInputAction(MyInputActionsData, InputAction);
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
		if(InputAction < EInputAction::MinPureUI)
		{
			ServerRPC_HandleAction(InputAction);
		}
		
		// input action prediction tbd. here
		
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
	UMyInputActionsData* MyInputActionsData;

	UPROPERTY()
    TObjectPtr<UEnhancedInputLocalPlayerSubsystem> Input;
};
