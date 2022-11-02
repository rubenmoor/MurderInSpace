// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LocalPlayer.h"
#include "MyLocalPlayer.generated.h"

UENUM(BlueprintType)
enum class ECurrentLevel : uint8
{
	MainMenu UMETA(DisplayName="MainMenu"),
	SpaceFootball UMETA(DisplayName="Spacefootball")
};

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UMyLocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	bool GetIsInMainMenu()
	{
		return CurrentLevel == ECurrentLevel::MainMenu;
	}

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ECurrentLevel CurrentLevel;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool IsMultiplayer;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool ShowInGameMenu;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool IsLoggedIn;
};
