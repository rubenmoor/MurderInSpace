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
	bool CreateSession(FUniqueNetIdRepl UNI, struct FHostSessionConfig SessionConfig, TFunctionRef<void(FName, bool)> Callback);
	bool DestroySession(TFunctionRef<void(FName, bool)> Callback);
	bool StartSession(TFunctionRef<void(FName, bool)> Callback);
	bool FindSessions(FUniqueNetIdRepl UNI, TFunctionRef<void(bool)> Callback);

	// show the login browser window for EOS
	// on a successful login, the Unique Net Id "OldUNI" will be replaced by a new one from the online subsystem
	void ShowLoginScreen(FUniqueNetIdRepl OldUNI);
	void BindOnLogout(TFunctionRef<void(int32, bool)> Handler);

	TArray<FOnlineSessionSearchResult> GetSearchResult() const { return LastSessionSearch->SearchResults; }

protected:
	// event handlers

private:
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

	IOnlineSessionPtr GetSessionInterface() const;
};
