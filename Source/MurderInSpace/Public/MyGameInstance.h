// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

UENUM(BlueprintType)
enum class EInstanceState : uint8
{
	StartUp UMETA(DisplayName="StartUp"),
	MenuMain UMETA(DisplayName="MenuMain"),
	MenuServers UMETA(DisplayName="MenuServers"),
	Loading UMETA(DisplayName="Loading"),
	MsgError UMETA(DisplayName="MsgError"),
	Playing UMETA(DisplayName="Playing"),
	Indeterminate UMETA(DisplayName="Indeterminate")
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
	void HostGame();
	
	UFUNCTION(BlueprintCallable)
	void ShowServers();

	UFUNCTION(BlueprintCallable)
	void ShowMainMenu();
	
	UFUNCTION(BlueprintCallable)
	void JoinGame();

	UFUNCTION(BlueprintCallable)
	void DestroySession();

	UFUNCTION(BlueprintCallable)
	void SetHUD();

	UFUNCTION(BlueprintCallable)
	void QuitGame();
	
protected:
	// event handlers

	UPROPERTY(BlueprintReadWrite)
	FRandomStream Random;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EInstanceState InstanceState = EInstanceState::StartUp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsEnabledLAN;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int TeamId;
};
