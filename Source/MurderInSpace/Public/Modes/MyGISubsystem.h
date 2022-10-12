// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MyGISubsystem.generated.h"

#define SETTING_CUSTOMNAME FName(TEXT("CUSTOMNAME"))

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UMyGISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	bool CreateSession(struct FHostSessionConfig SessionConfig, TFunctionRef<void(FName, bool)> Callback);
	bool DestroySession(TFunctionRef<void(FName, bool)> Callback);
	bool StartSession(TFunctionRef<void(FName, bool)> Callback);
	bool FindSessions(TFunctionRef<void(bool)> Callback);
	void ShowLoginScreen();

	TArray<FOnlineSessionSearchResult> GetSearchResult() const { return LastSessionSearch->SearchResults; }

protected:
	// event handlers

private:
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

	IOnlineSessionPtr GetSessionInterface() const;
};
