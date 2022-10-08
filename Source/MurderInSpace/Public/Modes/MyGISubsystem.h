// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MyGISubsystem.generated.h"

#define SETTING_CUSTOMNAME FName(TEXT("CUSTOMNAME"))

/*
 * minimal session setup information
 */
USTRUCT(BlueprintType)
struct FSessionConfig
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString CustomName;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int NumConnections;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bPrivate;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bEnableLAN;
};


/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UMyGISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	bool CreateSession(FSessionConfig SessionConfig, TFunctionRef<void(FName, bool)> Callback);
	bool DestroySession(TFunctionRef<void(FName, bool)> Callback);
	bool StartSession(TFunctionRef<void(FName, bool)>);
	bool FindSessions(TFunctionRef<void(FName, bool)>);

	TArray<FOnlineSessionSearchResult> GetSearchResult() { return LastSessionSearch->SearchResults; }

protected:
	// event handlers

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

private:
	IOnlineSessionPtr SI;

	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;
	
	FDelegateHandle DHCreateSession;
	FDelegateHandle DHDestroySession;
	FDelegateHandle DHStartSession;
	FDelegateHandle DHFindSessions;
};
