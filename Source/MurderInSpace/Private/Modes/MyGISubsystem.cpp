// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MyGISubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"

// TODO: Maybe FOnGISession...
ESIResult UMyGISubsystem::CreateSession(int NumPublicConnections, bool bIsLanMatch, TFunctionRef<void(FName, bool)> Callback)
{
	const IOnlineSessionPtr SI = Online::GetSessionInterface(GetWorld());
	if(!SI.IsValid())
	{
		return ESIResult::NoSessionInterface;
	}
	
	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->NumPrivateConnections = 0;
	LastSessionSettings->NumPublicConnections = NumPublicConnections;
	LastSessionSettings->bAllowInvites = true;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->bAllowJoinViaPresenceFriendsOnly = true;
	LastSessionSettings->bIsDedicated = false;
	LastSessionSettings->bUsesPresence = true;
	LastSessionSettings->bIsLANMatch = bIsLanMatch;
	LastSessionSettings->bShouldAdvertise = true;

	LastSessionSettings->Set(SETTING_MAPNAME, FString(TEXT("Space Football")), EOnlineDataAdvertisementType::ViaOnlineService);

	const TObjectPtr<ULocalPlayer> LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	// TODO: what is the cached unique net id logic?
	if (!SI->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings))
	{
		return ESIResult::Failure;
	}
	FOnCreateSessionComplete OnCreateSessionComplete;
	DHCreateSession = OnCreateSessionComplete.AddLambda([this, Callback, &OnCreateSessionComplete] (FName SessionName, bool bSuccess)
	{
		if(bSuccess)
		{
			switch(StartSession(Callback))
			{
			case ESIResult::NoSessionInterface:
				UE_LOG(LogNet, Error, TEXT("%s: couldn't get session interface"), *GetFullName())
				// fall through
			case ESIResult::Failure:
				UE_LOG(LogNet, Error, TEXT("%s: couldn't create session"), *GetFullName())
				Callback(SessionName, false);
				break;
			case ESIResult::Success:
				;
			}
		}
		OnCreateSessionComplete.Remove(DHCreateSession);
	});
	return ESIResult::Success;
}

ESIResult UMyGISubsystem::DestroySession(TFunctionRef<void(FName, bool)> Callback)
{
	const IOnlineSessionPtr SI = Online::GetSessionInterface(GetWorld());
	if (!SI.IsValid())
	{
		return ESIResult::NoSessionInterface;
	}
	if (!SI->DestroySession(NAME_GameSession))
	{
		return ESIResult::Failure;
	}
	FOnDestroySessionComplete OnDestroySessionComplete;
	DHDestroySession = OnDestroySessionComplete.AddLambda([this, Callback, &OnDestroySessionComplete] (FName SessionName, bool bSuccess)
	{
		Callback(SessionName, bSuccess);
		OnDestroySessionComplete.Remove(DHDestroySession);
	});
	return ESIResult::Success;
}

ESIResult UMyGISubsystem::StartSession(TFunctionRef<void(FName, bool)> Callback)
{
	const IOnlineSessionPtr SI = Online::GetSessionInterface(GetWorld());
	if (!SI.IsValid())
	{
		return ESIResult::NoSessionInterface;
	}
	if (!SI->StartSession(NAME_GameSession))
	{
		return ESIResult::Failure;
	}
	FOnStartSessionComplete OnStartSessionComplete;
	DHStartSession = OnStartSessionComplete.AddLambda([this, Callback, &OnStartSessionComplete] (FName SessionName, bool bSuccess)
	{
		Callback(SessionName, bSuccess);
		OnStartSessionComplete.Remove(DHStartSession);
	});
	return ESIResult::Success;
}

