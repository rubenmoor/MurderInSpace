// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class SPACESCENE_ARGHANION_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

	virtual void SetupInputComponent() override;

protected:
	void MouseMove(FVector VecDelta);
	void MouseWheel(float Delta);
	
	void MouseMoveX(float Delta);
	void MouseMoveY(float Delta);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 CameraPosition;

	static constexpr uint8 MaxCameraPosition = 8;
private:
	void HandleMouseMove(float Delta) const;
};
