// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

UENUM(BlueprintType)
enum class EInstanceState : uint8
{
	StartUp UMETA(DisplayName="startup"),
	InMenuMain UMETA(DisplayName="in main menu"),
	InMenuServers UMETA(DisplayName="in menu server list"),
	Loading UMETA(DisplayName="loading"),
	MsgError UMETA(DisplayName="error message"),
	InGame UMETA(DisplayName="in game"),
	InGameMenu UMETA(DisplayName="in game menu"),
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
	void HostGame();
	
	UFUNCTION(BlueprintCallable)
	void GotoInMenuServers();

	UFUNCTION(BlueprintCallable)
	void JoinGame();

	UFUNCTION(BlueprintCallable)
	void GotoInMenuMain();

	UFUNCTION(BlueprintCallable)
	void GotoInGame();

	UFUNCTION(BlueprintCallable)
	void GotoInGameMenu();

	UFUNCTION(BlueprintCallable)
	void QuitGame();
	
protected:
	// event handlers

	// private properties
	
	UPROPERTY(BlueprintReadWrite)
	FRandomStream Random;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EInstanceState InstanceState = EInstanceState::StartUp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsEnabledLAN;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int TeamId;

	// private methods
	UFUNCTION(BlueprintCallable)
	void ErrorWrongState(const FString& InStatesExpected);
};
