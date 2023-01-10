// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MyGameMode.generated.h"

/**
 *  the `game mode` is server-side data only
 */
UCLASS()
class MURDERINSPACE_API AMyGameMode : public AGameMode
{
	GENERATED_BODY()

protected:
	// event handlers
	// prelogin
	// postlogin
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
};
