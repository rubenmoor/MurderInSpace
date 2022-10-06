// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MenuGameMode.generated.h"

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API AMenuGameMode : public AGameMode
{
	GENERATED_BODY()

protected:
	// event handlers

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
};
