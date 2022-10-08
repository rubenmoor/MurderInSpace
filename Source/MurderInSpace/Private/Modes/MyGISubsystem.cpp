// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MyGISubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "Chaos/ChaosPerfTest.h"
#include "Modes/MyGameInstance.h"

bool UMyGISubsystem::CreateSession(FSessionConfig SessionConfig, TFunctionRef<void(FName, bool)> Callback)
{
	UE_LOG
		( LogNet
		, Warning
		, TEXT("%s: subsystem name: %s")
		, *GetFullName()
		, *IOnlineSubsystem::Get()->GetSubsystemName().ToString()
		)
	auto [ CustomName, NumConnections, bPrivate, bEnableLAN ] = SessionConfig;
	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->NumPrivateConnections = bPrivate ? NumConnections : 0;
	LastSessionSettings->NumPublicConnections = !bPrivate ? NumConnections : 0;
	LastSessionSettings->bAllowInvites = true;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->bAllowJoinViaPresenceFriendsOnly = true;
	LastSessionSettings->bIsDedicated = false;
	LastSessionSettings->bUsesPresence = true;
	LastSessionSettings->bIsLANMatch = bEnableLAN;
	LastSessionSettings->bShouldAdvertise = true;

	LastSessionSettings->Set(SETTING_MAPNAME, FString(TEXT("Space Football")), EOnlineDataAdvertisementType::ViaOnlineService);
	LastSessionSettings->Set(SETTING_CUSTOMNAME, FString(TEXT("Space Football")), EOnlineDataAdvertisementType::ViaOnlineService);

	if(SI->GetNamedSession(NAME_GameSession))
	{
		DestroySession([this, Callback, SessionConfig] (FName SessionName, bool bSuccess)
		{
			if(bSuccess)
			{
				// start over
				Cast<UMyGameInstance>(GetGameInstance())->HostGame(SessionConfig);
			}
			else
			{
				Callback(SessionName, false);
			}
		});
		// we pretend, it was a success so far, because this brings us to the
		// "waiting for session create" screen
		return true;
	}
	
	const TObjectPtr<ULocalPlayer> LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	// TODO: what is the cached unique net id logic?
	if (!SI->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings))
	{
		return false;
	}
	FOnCreateSessionComplete OnCreateSessionComplete;
	DHCreateSession = OnCreateSessionComplete.AddLambda([this, Callback, &OnCreateSessionComplete] (FName SessionName, bool bSuccess)
	{
		if(bSuccess)
		{
			if(!StartSession(Callback))
			{
				UE_LOG(LogNet, Error, TEXT("%s: couldn't create session"), *GetFullName())
				Callback(SessionName, false);
			}
		}
		OnCreateSessionComplete.Remove(DHCreateSession);
	});
	return true;
}

bool UMyGISubsystem::DestroySession(TFunctionRef<void(FName, bool)> Callback)
{
	if (!SI->DestroySession(NAME_GameSession))
	{
		return false;
	}
	FOnDestroySessionComplete OnDestroySessionComplete;
	DHDestroySession = OnDestroySessionComplete.AddLambda([this, Callback, &OnDestroySessionComplete] (FName SessionName, bool bSuccess)
	{
		Callback(SessionName, bSuccess);
		OnDestroySessionComplete.Remove(DHDestroySession);
	});
	return true;
}

bool UMyGISubsystem::StartSession(TFunctionRef<void(FName, bool)> Callback)
{
	if (!SI->StartSession(NAME_GameSession))
	{
		return false;
	}
	FOnStartSessionComplete OnStartSessionComplete;
	DHStartSession = OnStartSessionComplete.AddLambda([this, Callback, &OnStartSessionComplete] (FName SessionName, bool bSuccess)
	{
		Callback(SessionName, bSuccess);
		OnStartSessionComplete.Remove(DHStartSession);
	});
	return true;
}

bool UMyGISubsystem::FindSessions(TFunctionRef<void(FName, bool)> Callback)
{
	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = 128;
	LastSessionSearch->bIsLanQuery = Cast<UMyGameInstance>(GetGameInstance())->GetIsEnabledLAN();
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SI->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()))
	{
		return false;
	}
	FOnFindSessionsComplete OnFindSessionsComplete;
	DHFindSessions = OnFindSessionsComplete.AddLambda([this, Callback, &OnFindSessionsComplete] (FName SessionName, bool bSuccess)
	{
		Callback(SessionName, bSuccess);
		OnFindSessionsComplete.Remove(DHFindSessions);
	});
	return true;
}

void UMyGISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	SI = Online::GetSessionInterface(GetWorld());
	if(!SI.IsValid())
	{
		UE_LOG(LogNet, Error, TEXT("%s: couldn't get session interface"), *GetFullName())
	}
	
}

