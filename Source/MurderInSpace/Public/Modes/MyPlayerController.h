// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
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

	// input events
	
	// change the zoom level continuously
	void Zoom(float Delta);

	// given your current orientation, use the main rocket engine to accelerate
	UFUNCTION()
	void HandleBeginAccelerate();
	
	UFUNCTION()
	void HandleEndAccelerate();

	UFUNCTION()
	void HandleBeginShowMyTrajectory();

	UFUNCTION()
	void HandleEndShowMyTrajectory();
	
	UFUNCTION()
	void HandleBeginShowAllTrajectories();
	
	UFUNCTION()
	void HandleEndShowAllTrajectories();

	UFUNCTION(BlueprintCallable)
	void HandleEscape();

	UFUNCTION(Server, Reliable)
	void ServerRPC_LookAt(FQuat Quat);
	
private:
	void SetShowAllTrajectories(bool bInShow) const;
};
