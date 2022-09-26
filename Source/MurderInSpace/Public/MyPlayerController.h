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

	virtual void SetupInputComponent() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 CameraPosition = 2;

	static constexpr uint8 MaxCameraPosition = 8;

	// event handlers
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// input events
	
	// change the zoom level continuously
	void Zoom(float Delta);

	// given your current orientation, use the main rocket engine to accelerate
	UFUNCTION()
	void AccelerateBegin();
	
	UFUNCTION()
	void AccelerateEnd();

	// UFUNCTION()
	// void HandleSelect();
};
