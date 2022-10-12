// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MyGISubsystem.h"

#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "HUD/MyHUDMenu.h"
#include "Modes/MyGameInstance.h"
#include "Modes/MyPlayerState.h"

bool UMyGISubsystem::CreateSession(FHostSessionConfig SessionConfig, TFunctionRef<void(FName, bool)> Callback)
{
	const IOnlineSessionPtr SI = GetSessionInterface();
	UE_LOG
		( LogNet
		, Display
		, TEXT("%s: subsystem: %s; session interface is valid and unique: %s")
		, *GetFullName()
		, *IOnlineSubsystem::Get()->GetSubsystemName().ToString()
		, SI.IsUnique() ? TEXT("true") : TEXT("false")
		)
	
	auto [ CustomName, NumConnections, bPrivate, bEnableLAN, _GameMode ] = SessionConfig;
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
		UE_LOG
			( LogNet
			, Warning
			, TEXT("%s: Destroying existing session")
			, *GetFullName()
			)
		DestroySession([this, Callback] (FName SessionName, bool bSuccess)
		{
			if(bSuccess)
			{
				// start over
				Cast<UMyGameInstance>(GetGameInstance())->HostGame();
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
	
	FOnCreateSessionComplete OnCreateSessionComplete;
	OnCreateSessionComplete.AddLambda([this, Callback, &OnCreateSessionComplete] (FName SessionName, bool bSuccess)
	{
		if(bSuccess)
		{
			if(!StartSession(Callback))
			{
				UE_LOG(LogNet, Error, TEXT("%s: couldn't create session"), *GetFullName())
				Callback(SessionName, false);
			}
		}
		OnCreateSessionComplete.Clear();
	});

	const FUniqueNetIdRepl UniqueNetIdRepl = GetGameInstance()->GetPrimaryPlayerController()->GetPlayerState<AMyPlayerState>()->GetUniqueId();
	const TObjectPtr<ULocalPlayer> LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	// TODO: what is the cached unique net id logic?
	UE_LOG
		( LogOnline
		, Display
		, TEXT("%s: LocalPlayer->GetPreferredUniqueNetId(): %s, LocalPlayer->GetCachedUniqueNetId: %s; PlayerState->GetUniqueId: %s") 
		, *GetFullName()
		, *LocalPlayer->GetPreferredUniqueNetId().ToString()
		, *LocalPlayer->GetCachedUniqueNetId().ToString()
		, *UniqueNetIdRepl.ToString()
		)
	if (!SI->CreateSession(*UniqueNetIdRepl, NAME_GameSession, *LastSessionSettings))
	{
		return false;
	}
	return true;
}

bool UMyGISubsystem::DestroySession(TFunctionRef<void(FName, bool)> Callback)
{
	const IOnlineSessionPtr SI = GetSessionInterface();
	UE_LOG
		( LogNet
		, Display
		, TEXT("%s: subsystem: %s; session interface is valid and unique: %s")
		, *GetFullName()
		, *IOnlineSubsystem::Get()->GetSubsystemName().ToString()
		, SI.IsUnique() ? TEXT("true") : TEXT("false")
		)
	if (!SI->DestroySession(NAME_GameSession))
	{
		return false;
	}
	FOnDestroySessionComplete OnDestroySessionComplete;
	OnDestroySessionComplete.AddLambda([this, Callback, &OnDestroySessionComplete] (FName SessionName, bool bSuccess)
	{
		Callback(SessionName, bSuccess);
		OnDestroySessionComplete.Clear();
	});
	return true;
}

bool UMyGISubsystem::StartSession(TFunctionRef<void(FName, bool)> Callback)
{
	const IOnlineSessionPtr SI = GetSessionInterface();
	UE_LOG
		( LogNet
		, Display
		, TEXT("%s: subsystem: %s; session interface is valid and unique: %s")
		, *GetFullName()
		, *IOnlineSubsystem::Get()->GetSubsystemName().ToString()
		, SI.IsUnique() ? TEXT("true") : TEXT("false")
		)
	if (!SI->StartSession(NAME_GameSession))
	{
		return false;
	}
	FOnStartSessionComplete OnStartSessionComplete;
	OnStartSessionComplete.AddLambda([this, Callback, &OnStartSessionComplete] (FName SessionName, bool bSuccess)
	{
		Callback(SessionName, bSuccess);
		OnStartSessionComplete.Clear();
	});
	return true;
}

bool UMyGISubsystem::FindSessions(TFunctionRef<void(bool)> Callback)
{
	const IOnlineSessionPtr SI = GetSessionInterface();
	UE_LOG
		( LogNet
		, Display
		, TEXT("%s: subsystem: %s; session interface is valid and unique: %s")
		, *GetFullName()
		, *IOnlineSubsystem::Get()->GetSubsystemName().ToString()
		, SI.IsUnique() ? TEXT("true") : TEXT("false")
		)
	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = 128;
	LastSessionSearch->bIsLanQuery = Cast<UMyGameInstance>(GetGameInstance())->SessionConfig.bEnableLAN;
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SI->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()))
	{
		return false;
	}
	FOnFindSessionsComplete OnFindSessionsComplete;
	OnFindSessionsComplete.AddLambda([this, Callback, &OnFindSessionsComplete] (bool bSuccess)
	{
		Callback(bSuccess);
		OnFindSessionsComplete.Clear();
	});
	return true;
}

void UMyGISubsystem::ShowLoginScreen()
{
	FOnlineAccountCredentials OnlineAccountCredentials;
	// for epic games account
	//OnlineAccountCredentials.Type = "AccountPortal";

	// for DevAuthTool https://dev.epicgames.com/docs/epic-account-services/developer-authentication-tool
	OnlineAccountCredentials.Type = "Developer";
	OnlineAccountCredentials.Id = "localhost:1234";
	OnlineAccountCredentials.Token = "foo";
	const IOnlineIdentityPtr OSSIdentity = Online::GetIdentityInterfaceChecked(FName(TEXT("EOS")));
	AMyHUDMenu* HUDMenu = GetGameInstance()->GetPrimaryPlayerController()->GetHUD<AMyHUDMenu>();
	OSSIdentity->OnLoginCompleteDelegates->Clear();
	OSSIdentity->OnLoginCompleteDelegates->AddLambda([this, HUDMenu] (int32 Num, bool bSuccess, const FUniqueNetId& UniqueNetId, const FString& Error)
	{
		if(bSuccess)
		{
			UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance());
			GI->bLoggedIn = true;
			GI->GetPrimaryPlayerController()->GetPlayerState<AMyPlayerState>()->SetUniqueId(FUniqueNetIdRepl(UniqueNetId));
			GI->SessionConfig.bEnableLAN = false;
			HUDMenu->MenuMultiplayerShow();
		}
		else
		{
			HUDMenu->MessageShow
				( FText::FromString(Error)
				, [HUDMenu] () { HUDMenu->MenuMainShow(); }
				);
		}
	});
	OSSIdentity->OnLogoutCompleteDelegates->Clear();
	OSSIdentity->OnLogoutCompleteDelegates->AddLambda([this] (int32 Num, bool bSuccess)
	{
		Cast<UMyGameInstance>(GetGameInstance())->bLoggedIn = false;
	});
	OSSIdentity->Login(0, OnlineAccountCredentials);
}

IOnlineSessionPtr UMyGISubsystem::GetSessionInterface() const
{
	return Online::GetSessionInterfaceChecked
		( GetWorld()
		, Cast<UMyGameInstance>(GetGameInstance())->SessionConfig.bEnableLAN
			? FName(TEXT("NULL"))
			: FName(TEXT("EOS"))
		);
}
