// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

UENUM()
enum class EAction : uint8
{
	// given your current orientation, use the main rocket engine to accelerate
	ACCELERATE_BEGIN UMETA(DisplayName = "begin accelerate"),
	ACCELERATE_END   UMETA(DisplayName = "end accelerate")
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

	UFUNCTION(Server, Reliable)
	void ServerRPC_HandleAction(EAction Action);

	// handle the action: only the stuff that is relevant for replication 
	void HandleAction(EAction Action);

	// handle the action: only the stuff that is *NOT* relevant for replication, i.e. UI-related stuff
	void HandleActionUI(EAction Action);
};
