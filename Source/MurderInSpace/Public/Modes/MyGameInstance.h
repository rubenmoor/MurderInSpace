// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

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

	void HostGame(const FLocalPlayerContext& LPC);

	void LeaveGame(const FLocalPlayerContext& LPC);
	
	void StartSoloGame(const FLocalPlayerContext& LPC);

	void JoinGame(const FLocalPlayerContext& LPC);
	
	void QuitGame(const FLocalPlayerContext& LPC);

	FHostSessionConfig SessionConfig =
		{ ""
		, 4
		, false
		, false
		, EGameMode::EveryManForHimself
		};
protected:
	// event handlers

	virtual int32 AddLocalPlayer(ULocalPlayer* NewPlayer, int32 ControllerId) override;
	
	// private properties
	
	UPROPERTY(BlueprintReadWrite)
	FRandomStream Random;
};
