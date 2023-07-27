// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MySessionManager.h"

#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "HUD/MyHUDMenu.h"
#include "Logging/StructuredLog.h"
#include "Modes/MyGameInstance.h"
#include "Modes/MyLocalPlayer.h"
#include "Modes/MyPlayerState.h"
#include "Online/OnlineSessionNames.h"

#define LOCTEXT_NAMESPACE "Menu"

bool UMySessionManager::CreateSession(const FLocalPlayerContext& LPC, FHostSessionConfig SessionConfig, std::function<void(FName, bool)> Callback)
{
	const IOnlineSessionPtr SI = GetSessionInterface();
	
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
	LastSessionSettings->Set(SETTING_CUSTOMNAME, CustomName, EOnlineDataAdvertisementType::ViaOnlineService);

	if(SI->GetNamedSession(NAME_GameSession))
	{
		UE_LOGFMT
			( LogNet
			, Warning
			, "{}: Destroying existing session"
			, GetFName()
			);
		LeaveSession([this, Callback, LPC] (FName SessionName, bool bSuccess)
		{
			if(bSuccess)
			{
				// start over
				Cast<UMyGameInstance>(GetGameInstance())->HostGame(LPC);
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

	if(SI->OnCreateSessionCompleteDelegates.IsBound())
	{
		UE_LOGFMT(LogNet, Warning, "{}: OnCreateSessionCompleteDelegates: was bound, clearing", GetFName());
		SI->OnCreateSessionCompleteDelegates.Clear();
	}
	// debugging: create session without subsequent call to StartSession
	//SI->OnCreateSessionCompleteDelegates.AddLambda(Callback);
	
	// debugging: create session and call StartSession on callback
	SI->OnCreateSessionCompleteDelegates.AddLambda([this, Callback] (FName SessionName, bool bSuccess)
	{
		if(bSuccess)
		{
			 if(!StartSession(Callback))
			 {
			 	UE_LOGFMT(LogNet, Error, "{}: couldn't start session", GetFName());
			 	Callback(SessionName, false);
			 }
		}
		else
		{
			Callback(SessionName, false);
		}
	});

	//return SI->CreateSession(*LPC.GetLocalPlayer()->GetCachedUniqueNetId(), NAME_GameSession, *LastSessionSettings);
	return SI->CreateSession(LPC.GetLocalPlayer()->GetIndexInGameInstance(), NAME_GameSession, *LastSessionSettings);
}

void UMySessionManager::LeaveSession(std::function<void(FName, bool)> Callback)
{
	const IOnlineSessionPtr SI = GetSessionInterface();
	if(SI->GetNamedSession(NAME_GameSession))
	{
		if(SI->OnDestroySessionCompleteDelegates.IsBound())
		{
			UE_LOGFMT(LogNet, Warning, "{}: OnDestroySessionCompleteDelegates: was bound, clearing", GetFName());
			SI->OnDestroySessionCompleteDelegates.Clear();
		}
		SI->OnDestroySessionCompleteDelegates.AddLambda([this, SI, Callback] (FName SessionName, bool bSuccess)
		{
			// `DestroySession` does seem to have some glitches, where the session ends up not being destroyed.
			// Unfortunately, I regularly encounter the case where `bSuccess` is true, but the session isn't destroyed.
			if(SI->GetNamedSession(NAME_GameSession))
			{
				UE_LOGFMT(LogTemp, Warning, "{}: Failed to destroy session, trying again ...", GetFName());
				SI->DestroySession(NAME_GameSession);
			}
			else
			{
				UE_LOGFMT(LogTemp, Warning, "{}: Session destroyed.", GetFName());
				Callback(SessionName, bSuccess);
			}
		});
		SI->DestroySession(NAME_GameSession);
	}
}

void UMySessionManager::LeaveSession()
{
	LeaveSession([this] (FName, bool)
	{
		GetGameInstance()->ReturnToMainMenu();
	});
}

bool UMySessionManager::StartSession(std::function<void(FName, bool)> Callback)
{
	const IOnlineSessionPtr SI = GetSessionInterface();
	if(SI->OnStartSessionCompleteDelegates.IsBound())
	{
		UE_LOGFMT(LogNet, Warning, "{}: OnStartSessionCompleteDelegates: was bound, clearing", GetFName());
		SI->OnStartSessionCompleteDelegates.Clear();
	}
	SI->OnStartSessionCompleteDelegates.AddLambda(Callback);
	return SI->StartSession(NAME_GameSession);
}

bool UMySessionManager::FindSessions(const FLocalPlayerContext& LPC, std::function<void(bool)> Callback)
{
	const IOnlineSessionPtr SI = GetSessionInterface();
	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = 10000;
	LastSessionSearch->bIsLanQuery = Cast<UMyGameInstance>(GetGameInstance())->SessionConfig.bEnableLAN;
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	if(SI->OnFindSessionsCompleteDelegates.IsBound())
	{
		UE_LOGFMT(LogNet, Warning, "{}: OnFindSessionsCompleteDelegates: was bound, clearing", GetFName());
		SI->OnFindSessionsCompleteDelegates.Clear();
	}
	SI->OnFindSessionsCompleteDelegates.AddLambda(Callback);
	return SI->FindSessions(LPC.GetLocalPlayer()->GetIndexInGameInstance(), LastSessionSearch.ToSharedRef());
}

bool UMySessionManager::JoinSession(const FLocalPlayerContext& LPC, const FOnlineSessionSearchResult& Result, std::function<void(FName, EOnJoinSessionCompleteResult::Type)> Callback)
{
	const IOnlineSessionPtr SI = GetSessionInterface();
	if(SI->GetNamedSession(NAME_GameSession))
	{
		UE_LOGFMT
			( LogNet
			, Warning
			, "{}: Destroying existing session"
			, GetFName()
			);
		LeaveSession([this, Callback, LPC, Result] (FName SessionName, bool bSuccess)
		{
			if(bSuccess)
			{
				// start over
				Cast<UMyGameInstance>(GetGameInstance())->JoinSession(LPC.GetLocalPlayer(), Result);
			}
			else
			{
				Callback(SessionName, EOnJoinSessionCompleteResult::UnknownError);
			}
		});
		// we pretend, it was a success so far, because this brings us to the
		// "waiting for session create" screen
		return true;
	}
	
	if(SI->OnJoinSessionCompleteDelegates.IsBound())
	{
		UE_LOGFMT(LogNet, Warning, "{}: OnJoinSessionCompleteDelegates: was bound, clearing", GetFName());
		SI->OnJoinSessionCompleteDelegates.Clear();
	}
	SI->OnJoinSessionCompleteDelegates.AddLambda(Callback);
	Cast<UMyLocalPlayer>(LPC.GetLocalPlayer())->InGame = EInGame::IngameJoining;
	return SI->JoinSession(LPC.GetLocalPlayer()->GetIndexInGameInstance(), NAME_GameSession, Result);
}

void UMySessionManager::ShowLoginScreen(const FLocalPlayerContext& LPC)
{
	FOnlineAccountCredentials OnlineAccountCredentials;
	
	// for epic games account
	//OnlineAccountCredentials.Type = "AccountPortal";

	// for DevAuthTool https://dev.epicgames.com/docs/epic-account-services/developer-authentication-tool
	OnlineAccountCredentials.Type = "Developer";
	OnlineAccountCredentials.Id = "localhost:1234";
	OnlineAccountCredentials.Token = "foo";

	const IOnlineIdentityPtr OSSIdentity = Online::GetIdentityInterfaceChecked(GetWorld(), "EOS");
	
	OSSIdentity->Login
		( LPC.GetLocalPlayer()->GetLocalPlayerIndex()
		, OnlineAccountCredentials
		);
	
	// TODO: autologin for PIE
	
	LPC.GetHUD<AMyHUDMenu>()->LoadingScreenShow
		(LOCTEXT("WaitingForLogin...", "Waiting for log-in to complete ...")
		, [LPC] ()
		{
			LPC.GetHUD<AMyHUDMenu>()->MenuMainShow();
		});
}

void UMySessionManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const IOnlineIdentityPtr OSSIdentity = Online::GetIdentityInterfaceChecked(GetWorld(), "EOS");
	
	// login handler
	
	if(OSSIdentity->OnLoginCompleteDelegates->IsBound())
	{
		UE_LOGFMT(LogNet, Warning, "{}: OnLoginCompleteDelegates: was bound, clearing", GetFName());
		OSSIdentity->OnLoginCompleteDelegates->Clear();
	}
	OSSIdentity->OnLoginCompleteDelegates->AddLambda([this] (int32 LocalUserNum, bool bSuccess, const FUniqueNetId& NewUNI, const FString& Error)
	{
		UE_LOGFMT
			( LogNet
			, Display
			, "{}: Login of player num {}: {}"
			, GetFName()
			, LocalUserNum
			, bSuccess
			);

		UMyLocalPlayer* LocalPlayer = Cast<UMyLocalPlayer>(GetGameInstance()->GetLocalPlayerByIndex(LocalUserNum));
		AMyHUDMenu* HUDMenu = LocalPlayer->GetPlayerController(GetWorld())->GetHUD<AMyHUDMenu>();
		
		if(bSuccess)
		{
			LocalPlayer->IsLoggedIn = true;
			LocalPlayer->SetCachedUniqueNetId(FUniqueNetIdRepl(NewUNI));
			
			Cast<UMyGameInstance>(GetGameInstance())->SessionConfig.bEnableLAN = false;
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
	
	// logout handler
	
	if(OSSIdentity->OnLogoutCompleteDelegates->IsBound())
	{
		UE_LOGFMT(LogNet, Warning, "{}: OnLogoutCompleteDelegates: was bound, clearing", GetFName());
		OSSIdentity->OnLogoutCompleteDelegates->Clear();
	}
	OSSIdentity->OnLogoutCompleteDelegates->AddLambda([this] (int32 PlayerNum, bool bSuccess)
	{
		if(bSuccess)
		{
			Cast<UMyLocalPlayer>(GetGameInstance()->GetLocalPlayerByIndex(PlayerNum))->IsLoggedIn = false;
		}
		UE_LOGFMT
			( LogNet
			, Display
			, "{}: Player {} log out: {}"
			, GetFName()
			, PlayerNum
			, bSuccess
			);
	});
}

IOnlineSessionPtr UMySessionManager::GetSessionInterface() const
{
	return Online::GetSessionInterfaceChecked
		( GetWorld()
		, Cast<UMyGameInstance>(GetGameInstance())->SessionConfig.bEnableLAN
			? "NULL"
			: "EOS"
		);
}
#undef LOCTEXT_NAMESPACE
