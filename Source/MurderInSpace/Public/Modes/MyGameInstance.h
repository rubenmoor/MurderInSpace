// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Lib/UStateLib.h"
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

	//void LeaveGame(const FLocalPlayerContext& LPC);
	
	void StartSoloGame(const FLocalPlayerContext& LPC);

	virtual bool JoinSession(ULocalPlayer* LocalPlayer, const FOnlineSessionSearchResult& SearchResult) override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_LeaveSession();
	
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

	UPROPERTY(BlueprintReadWrite)
	FInstanceUI InstanceUI = UStateLib::DefaultInstanceUI;
	
// custom event handlers
public:
	// clicking on pawns and actors impacts the visibility of their respective orbits
	// so instead of implementing the following handlers twice, in ActorInSpace and PawnInSpace,
	// we put them here
	UFUNCTION(BlueprintCallable)
	void HandleBeginMouseOver(AActor* Actor);
	
	UFUNCTION(BlueprintCallable)
	void HandleEndMouseOver(AActor* Actor);
	
	UFUNCTION(BlueprintCallable)
	void HandleClick(AActor* Actor, FKey Button);
};
