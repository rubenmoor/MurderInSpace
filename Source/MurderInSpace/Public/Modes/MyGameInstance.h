#pragma once

#include "CoreMinimal.h"
#include "MyState.h"
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
	int32 NumMaxPlayers = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bPrivate = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bEnableLAN = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	EGameMode GameMode = EGameMode::Coop;
};

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

	friend class UMyState;

public:
	UMyGameInstance();

	static UMyGameInstance* Get(const UWorld* World) { return World->GetGameInstance<UMyGameInstance>(); }

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

	void IncGlobalNumSplinePoints(int Amount)
	{
		GlobalNumSplinePoints += Amount;
		UE_LOG(LogMyGame, Display, TEXT("Global number of spline points: %d"), GlobalNumSplinePoints)
	}

	UPROPERTY(BlueprintReadWrite)
	FInstanceUI InstanceUI;
	
protected:
	// event handlers

	virtual int32 AddLocalPlayer(ULocalPlayer* NewPlayer, FPlatformUserId UserId) override;
	
	// private properties
	int GlobalNumSplinePoints = 0;
	
	// flag enums
};
