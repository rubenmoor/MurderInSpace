// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MyGameMode.generated.h"

/**
 *  the `game mode` is server-side data only
 *  I don't yet understand multiplayer setup and thus shouldn't put anything here, yet
 */
UCLASS()
class MURDERINSPACE_API AMyGameMode : public AGameMode
{
	GENERATED_BODY()

protected:
	// event handlers
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
};
