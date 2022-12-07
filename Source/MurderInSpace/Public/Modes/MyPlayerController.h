// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputMappingContext.h"
#include "MyLocalPlayer.h"
#include "Actors/MyCharacter.h"
#include "Actors/Orbit.h"
#include "GameFramework/PlayerController.h"
#include "HUD/MyHUD.h"
#include "Input/MyEnhancedInputComponent.h"
#include "MyPlayerController.generated.h"

UENUM()
enum class EAction : uint8
{
	// given your current orientation, use the main rocket engine to accelerate
	ACCELERATE_BEGIN UMETA(DisplayName = "begin accelerate"),
	ACCELERATE_END   UMETA(DisplayName = "end accelerate")
};

UENUM()
enum class EPureUIAction : uint8
{
	// given your current orientation, use the main rocket engine to accelerate
	  ToggleIngameMenu UMETA(DisplayName = "toggle in-game menu")
	, ShowMyTrajectory UMETA(DisplayName = "show my trajectory")
	, HideMyTrajectory UMETA(DisplayName = "hide my trajectory")
	, ShowAllTrajectories UMETA(DisplayName = "show all trajectories")
	, HideAllTrajectories UMETA(DisplayName = "hide all trajectories")
	, ToggleMyTrajectory UMETA(DisplayName = "toggle my trajectories visibility")
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
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 CameraPosition = 2;

	static constexpr uint8 MaxCameraPosition = 8;

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
	void SetShowAllTrajectories(bool bInShow) const;

	// custom way to bind actions using the enum `EAction` and only one generic Server RPC
	void BindEAction(const FName ActionName, EInputEvent KeyEvent, EAction Action);

	void BindInputLambda(const FName ActionName, EInputEvent KeyEvent, std::function<void()> Handler);

	template<EPureUIAction PureUIAction>
	void BindPureUIAction(FGameplayTag GameplayTag)
	{
		Cast<UMyEnhancedInputComponent>(InputComponent)->BindActionByTag
			( MyInputActionsData
			, GameplayTag
			, ETriggerEvent::Triggered
			, this
			, &AMyPlayerController::HandlePureUIAction<PureUIAction>
			);
	}

	// pure UI actions
	template<EPureUIAction PureUIAction>
	void HandlePureUIAction()
	{
		UMyLocalPlayer* LocalPlayer = Cast<UMyLocalPlayer>(Player);
		UMyState* MyState = GEngine->GetEngineSubsystem<UMyState>();
		AOrbit* Orbit;
		switch(PureUIAction)
		{
		case EPureUIAction::ToggleIngameMenu:
			if(LocalPlayer->ShowInGameMenu)
			{
				GetHUD<AMyHUD>()->InGameMenuHide();
				CurrentMouseCursor = EMouseCursor::Crosshairs;
				LocalPlayer->ShowInGameMenu = false;
			}
			else
			{
				GetHUD<AMyHUD>()->InGameMenuShow();
				CurrentMouseCursor = EMouseCursor::Default;
				LocalPlayer->ShowInGameMenu = true;
			}
			break;
		case EPureUIAction::ShowMyTrajectory:
            Orbit = Cast<AOrbit>(GetPawn<AMyCharacter>()->Children[0]);
            Orbit->bIsVisibleVarious = true;
            Orbit->UpdateVisibility(MyState->GetInstanceUIAny(this));
			break;
		case EPureUIAction::HideMyTrajectory:
            Orbit = Cast<AOrbit>(GetPawn<AMyCharacter>()->Children[0]);
            Orbit->bIsVisibleVarious = false;
            Orbit->UpdateVisibility(MyState->GetInstanceUIAny(this));
			break;
		case EPureUIAction::ShowAllTrajectories:
            SetShowAllTrajectories(true);
			break;
		case EPureUIAction::HideAllTrajectories:
            SetShowAllTrajectories(false);
			break;
		case EPureUIAction::ToggleMyTrajectory:
			// TODO
			break;
		}
	}
	
	// gameplay actions
	UFUNCTION(Server, Reliable)
	void ServerRPC_HandleAction(EAction Action);

	// handle the action: only the stuff that is relevant for replication 
	void HandleAction(EAction Action);

	// handle the action: only the stuff that is *NOT* relevant for replication, i.e. UI-related stuff
	void HandleActionUI(EAction Action);

	// private members

	// input mapping context: in-game
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UInputMappingContext> IMC_InGame;

	UPROPERTY(VisibleAnywhere)
	class UMyInputActionsData* MyInputActionsData;
};
