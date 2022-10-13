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
	WaitingForStart UMETA(DisplayName="waiting for session create"),
	MsgError UMETA(DisplayName="error message"),
	InGame UMETA(DisplayName="in game"),
	Indeterminate UMETA(DisplayName="status indeterminate")
};

UENUM(BlueprintType)
enum class EGameMode : uint8
{
	EveryManForHimself UMETA(DisplayName="every man for himself"),
	Teams UMETA(DisplayName="teams"),
	Coop UMETA(DisplayName="coop"),
};

/*
 * minimal session setup information
 */
USTRUCT(BlueprintType)
struct FHostSessionConfig
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString CustomName;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 NumMaxPlayers;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bPrivate;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bEnableLAN;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	EGameMode GameMode;
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
	int32 GetLocalPlayerIndex(FUniqueNetIdRepl UNI);
	
	UFUNCTION(BlueprintCallable)
	bool GetIsMultiplayer() { return bIsMultiplayer; }
	
	UFUNCTION(BlueprintCallable)
	bool GetShowInGameMenu() { return bShowInGameMenu; }

	UFUNCTION(BlueprintCallable)
	void HostGame(const APlayerController* PC);

	UFUNCTION(BlueprintCallable)
	void StartSoloGame(const APlayerController* PC);

	UFUNCTION(BlueprintCallable)
	void JoinGame();
	
	UFUNCTION(BlueprintCallable)
	void GotoInMenuMain(const APlayerController* PC);

	UFUNCTION(BlueprintCallable)
	void GotoInGame(const APlayerController* PC);

	UFUNCTION(BlueprintCallable)
	void InGameMenuShow(const APlayerController* PC);

	UFUNCTION(BlueprintCallable)
	void InGameMenuHide(const APlayerController* PC);

	UFUNCTION(BlueprintCallable)
	void GotoWaitingForStart(const APlayerController* PC);

	UFUNCTION(BlueprintCallable)
	void QuitGame(APlayerController* PC);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bLoggedIn;

	FHostSessionConfig SessionConfig =
		{ ""
		, 4
		, false
		, false
		, EGameMode::EveryManForHimself
		};
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
	int TeamId;

	// private methods

	UFUNCTION(BlueprintCallable)
	void ErrorWrongState(const UObject* Object, const FString& InStatesExpected);
};
