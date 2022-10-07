// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MyGISubsystem.generated.h"

DECLARE_DELEGATE_OneParam(FOnGISessionCreatedSignature, bool /* bSuccess */);
DECLARE_DELEGATE_OneParam(FOnGISessionDestroyedSignature, bool /* bSuccess */);
DECLARE_DELEGATE_OneParam(FOnGISessionStartedSignature, bool /* bSuccess */);

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UMyGISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UMyGISubsystem();

	FOnGISessionCreatedSignature CreateSession(int NumPublicConnections, bool bIsLanMatch);
	
	FOnGISessionDestroyedSignature DestroySession();

	FOnGISessionStartedSignature StartSession();

protected:
	UFUNCTION()
	void HandleGISessionCrated(FName SessionName, bool bSuccess);

	UFUNCTION()
	void HandleGISessionDestroyed(FName SessionName, bool bSuccess);
	
	UFUNCTION()
	void HandleGISessionStarted(FName SessionName, bool bSuccess);
	
private:
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	
	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;

	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;

	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;
	FDelegateHandle StartSessionCompleteDelegateHandle;
	
	void WithSessionInterface(TFunctionRef<void(IOnlineSessionPtr)> Func) const;
};
