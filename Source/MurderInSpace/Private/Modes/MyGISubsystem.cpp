// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MyGISubsystem.h"

#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "GameFramework/GameSession.h"
#include "HUD/MyHUDMenu.h"
#include "Modes/MyGameInstance.h"
#include "Modes/MyPlayerController.h"
#include "Modes/MyPlayerState.h"

bool UMyGISubsystem::CreateSession(FUniqueNetIdRepl UNI, FHostSessionConfig SessionConfig, TFunctionRef<void(FName, bool)> Callback)
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
	LastSessionSettings->Set(SETTING_CUSTOMNAME, FString(TEXT("Space Football")), EOnlineDataAdvertisementType::ViaOnlineService);

	if(SI->GetNamedSession(NAME_GameSession))
	{
		UE_LOG
			( LogNet
			, Warning
			, TEXT("%s: Destroying existing session")
			, *GetFullName()
			)
		DestroySession([this, Callback, UNI] (FName SessionName, bool bSuccess)
		{
			if(bSuccess)
			{
				// start over
				Cast<UMyGameInstance>(GetGameInstance())->HostGame(GetPlayerControllerFromNetId(GetWorld(), UNI));
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
		UE_LOG(LogNet, Warning, TEXT("%s: OnCreateSessionCompleteDelegates: was bound, clearing"), *GetFullName())
		SI->OnCreateSessionCompleteDelegates.Clear();
	}
	SI->OnCreateSessionCompleteDelegates.AddLambda([this, Callback] (FName SessionName, bool bSuccess)
	{
		if(bSuccess)
		{
			if(!StartSession(Callback))
			{
				UE_LOG(LogNet, Error, TEXT("%s: couldn't start session"), *GetFullName())
				Callback(SessionName, false);
			}
		}
		else
		{
			Callback(SessionName, false);
		}
	});

	return SI->CreateSession(*UNI, NAME_GameSession, *LastSessionSettings);
}

bool UMyGISubsystem::DestroySession(TFunctionRef<void(FName, bool)> Callback)
{
	const IOnlineSessionPtr SI = GetSessionInterface();
	if(SI->OnDestroySessionCompleteDelegates.IsBound())
	{
		UE_LOG(LogNet, Warning, TEXT("%s: OnDestroySessionCompleteDelegates: was bound, clearing"), *GetFullName())
		SI->OnDestroySessionCompleteDelegates.Clear();
	}
	SI->OnDestroySessionCompleteDelegates.AddLambda(Callback);
	return SI->DestroySession(NAME_GameSession);
}

bool UMyGISubsystem::StartSession(TFunctionRef<void(FName, bool)> Callback)
{
	const IOnlineSessionPtr SI = GetSessionInterface();
	if(SI->OnStartSessionCompleteDelegates.IsBound())
	{
		UE_LOG(LogNet, Warning, TEXT("%s: OnStartSessionCompleteDelegates: was bound, clearing"), *GetFullName())
		SI->OnStartSessionCompleteDelegates.Clear();
	}
	SI->OnStartSessionCompleteDelegates.AddLambda(Callback);
	return SI->StartSession(NAME_GameSession);
}

bool UMyGISubsystem::FindSessions(FUniqueNetIdRepl UNI, TFunctionRef<void(bool)> Callback)
{
	const IOnlineSessionPtr SI = GetSessionInterface();
	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = 128;
	LastSessionSearch->bIsLanQuery = Cast<UMyGameInstance>(GetGameInstance())->SessionConfig.bEnableLAN;
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	if(SI->OnFindSessionsCompleteDelegates.IsBound())
	{
		UE_LOG(LogNet, Warning, TEXT("%s: OnFindSessionsCompleteDelegates: was bound, clearing"), *GetFullName())
		SI->OnFindSessionsCompleteDelegates.Clear();
	}
	SI->OnFindSessionsCompleteDelegates.AddLambda(Callback);
	return SI->FindSessions(*UNI, LastSessionSearch.ToSharedRef());
}

void UMyGISubsystem::ShowLoginScreen(FUniqueNetIdRepl OldUNI)
{
	FOnlineAccountCredentials OnlineAccountCredentials;
	
	// for epic games account
	//OnlineAccountCredentials.Type = "AccountPortal";

	// for DevAuthTool https://dev.epicgames.com/docs/epic-account-services/developer-authentication-tool
	OnlineAccountCredentials.Type = "Developer";
	OnlineAccountCredentials.Id = "localhost:1234";
	OnlineAccountCredentials.Token = "foo";
	
	const IOnlineIdentityPtr OSSIdentity = Online::GetIdentityInterfaceChecked(FName(TEXT("EOS")));
	
	if(OSSIdentity->OnLoginCompleteDelegates->IsBound())
	{
		UE_LOG(LogNet, Warning, TEXT("%s: OnLoginCompleteDelegates: was bound, clearing"), *GetFullName())
		OSSIdentity->OnLoginCompleteDelegates->Clear();
	}
	OSSIdentity->OnLoginCompleteDelegates->AddLambda([this] (int32 LocalUserNum, bool bSuccess, const FUniqueNetId& NewUNI, const FString& Error)
	{
		UE_LOG
			( LogNet
			, Display
			, TEXT("%s: Login of player num %d: %s")
			, *GetFullName()
			, LocalUserNum
			, bSuccess ? TEXT("success") : TEXT("failure")
			)

		// the unique net id may change, the player controller stays the same
		const APlayerController* PC = GetGameInstance()->GetLocalPlayerByIndex(LocalUserNum)->GetPlayerController(GetWorld());
		AMyHUDMenu* HUDMenu = PC->GetHUD<AMyHUDMenu>();
		
		if(bSuccess)
		{
			UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance());
			GI->bLoggedIn = true;
			PC->GetPlayerState<AMyPlayerState>()->SetUniqueId(FUniqueNetIdRepl(NewUNI));
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
	OSSIdentity->Login
		( Cast<UMyGameInstance>(GetGameInstance())->GetLocalPlayerIndex(OldUNI)
		, OnlineAccountCredentials
		);
}

// called by UW_MenuMain: when the user clicks on "multiplayer"
void UMyGISubsystem::BindOnLogout(TFunctionRef<void(int32, bool)> Handler)
{
	const IOnlineIdentityPtr OSSIdentity = Online::GetIdentityInterfaceChecked(FName(TEXT("EOS")));
	if(OSSIdentity->OnLogoutCompleteDelegates->IsBound())
	{
		UE_LOG(LogNet, Warning, TEXT("%s: OnLogoutCompleteDelegates: was bound, clearing"), *GetFullName())
		OSSIdentity->OnLogoutCompleteDelegates->Clear();
	}
	OSSIdentity->OnLogoutCompleteDelegates->AddLambda(Handler);
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
