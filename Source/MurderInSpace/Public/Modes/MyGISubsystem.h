// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MyGISubsystem.generated.h"

UENUM(BlueprintType)
enum class ESIResult : uint8
{
	Success UMETA(DisplayName="success"),
	NoSessionInterface UMETA(DisplayName="no session interface"),
	Failure UMETA(DisplayName="failure")
};

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UMyGISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	ESIResult CreateSession(int NumPublicConnections, bool bIsLanMatch, TFunctionRef<void(FName, bool)> Callback);
	ESIResult DestroySession(TFunctionRef<void(FName, bool)> Callback);
	ESIResult StartSession(TFunctionRef<void(FName, bool)>);

private:
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	FDelegateHandle DHCreateSession;
	FDelegateHandle DHDestroySession;
	FDelegateHandle DHStartSession;
};
