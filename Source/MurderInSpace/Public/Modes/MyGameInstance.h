// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

UENUM(BlueprintType)
enum class EInstanceState : uint8
{
	StartUp UMETA(DisplayName="startup"),
	InMainMenu UMETA(DisplayName="in main menu"),
	WaitingForSessionCreate UMETA(DisplayName="waiting for session create"),
	MsgError UMETA(DisplayName="error message"),
	InGame UMETA(DisplayName="in game"),
	Indeterminate UMETA(DisplayName="status indeterminate")
};

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

	friend class UStateLib;
public:
	UMyGameInstance();
	
	UFUNCTION(BlueprintCallable)
	EInstanceState GetInstanceState() { return InstanceState; }

	UFUNCTION(BlueprintCallable)
	bool GetIsMultiplayer() { return bIsMultiplayer; }
	
	UFUNCTION(BlueprintCallable)
	bool GetShowInGameMenu() { return bShowInGameMenu; }

	UFUNCTION(BlueprintCallable)
	bool GetIsEnabledLAN() { return bIsEnabledLAN; }
	
	UFUNCTION(BlueprintCallable)
	void HostGame(const FSessionConfig& SessionConfig);

	UFUNCTION(BlueprintCallable)
	void JoinGame();

	UFUNCTION(BlueprintCallable)
	void FindGames();

	UFUNCTION(BlueprintCallable)
	void GotoInMenuMain();

	UFUNCTION(BlueprintCallable)
	void GotoInGame();

	UFUNCTION(BlueprintCallable)
	void InGameMenuShow();

	UFUNCTION(BlueprintCallable)
	void InGameMenuHide();

	UFUNCTION(BlueprintCallable)
	void GotoWaitingForSessionCreate();

	UFUNCTION(BlueprintCallable)
	void QuitGame();
	
protected:
	// event handlers

	// private properties
	
	UPROPERTY(BlueprintReadWrite)
	FRandomStream Random;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EInstanceState InstanceState = EInstanceState::StartUp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bIsMultiplayer;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bShowInGameMenu;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsEnabledLAN;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int TeamId;

	// private methods

	UFUNCTION(BlueprintCallable)
	void ErrorWrongState(const UObject* Object, const FString& InStatesExpected);
};
