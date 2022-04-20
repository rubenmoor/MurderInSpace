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
	void MouseMove(FVector Delta);
};
