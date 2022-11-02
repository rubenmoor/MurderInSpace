// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GameMode_Menu.generated.h"

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API AGameMode_Menu : public AGameMode
{
	GENERATED_BODY()

protected:
	// event handlers

	virtual void BeginPlay() override;
};
