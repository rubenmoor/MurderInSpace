// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MyGameInstance.h"

#include "LoadingScreenModule.h"
#include "HUD/MyHUDMenu.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Logging/StructuredLog.h"
#include "Modes/MySessionManager.h"
#include "Modes/MyLocalPlayer.h"

#define LOCTEXT_NAMESPACE "Menu"

UMyGameInstance::UMyGameInstance()
{
}

void UMyGameInstance::HostGame(const FLocalPlayerContext& LPC)
{
	UMySessionManager* GISub = GetSubsystem<UMySessionManager>();
	AMyHUDMenu* HUDMenu = LPC.GetHUD<AMyHUDMenu>();
	
	// ReSharper disable once CppTooWideScope
	const bool bSuccess = GISub->CreateSession
		( LPC
		, SessionConfig
		, [this, LPC, HUDMenu] (FName SessionName, bool bSuccess)
		{
			if(bSuccess)
			{
				GetWorld()->ServerTravel("/Game/Maps/Spacefootball?listen");
				//UGameplayStatics::OpenLevel(GetWorld(), "spacefootball");
			}
			else
			{
				HUDMenu->MessageShow(LOCTEXT
					( "CreateSessionOnlineSubsystemFailure", "the online subsystem returned with failure")
					, [&LPC] () { LPC.GetHUD<AMyHUDMenu>()->MenuMainShow(); }
					);
				UE_LOGFMT(LogNet, Error, "{}: create session call returned with failure", GetFName());
			}
		});
	
	if(bSuccess)
	{
		Cast<UMyLocalPlayer>(LPC.GetLocalPlayer())->IsMultiplayer = true;
		HUDMenu->LoadingScreenShow
			( SessionConfig.bEnableLAN
				? LOCTEXT("CreatingLANSession", "creating local network (LAN) session ...")
				: LOCTEXT("CreatingOnlineSession", "creating online session ...")
			, [HUDMenu] () { HUDMenu->MenuMultiplayerShow(); }
			);
	}
	else
	{
		HUDMenu->MessageShow(LOCTEXT
			( "CreateSessionReturnedFailure", "call to create session returned failure" )
			, [HUDMenu] () { HUDMenu->MenuMainShow(); }
			);
		UE_LOGFMT(LogNet, Error, "{}: couldn't create session", GetFName());
	}
}

// void UMyGameInstance::LeaveGame(const FLocalPlayerContext& LPC)
// {
// 	UMyLocalPlayer* LocalPlayer = Cast<UMyLocalPlayer>(LPC.GetLocalPlayer());
// 	if(LocalPlayer->IsMultiplayer)
// 	{
// 		GetSubsystem<UMyGISubsystem>()->DestroySession([] (FName, bool) {} );
// 	}
// 	// TODO: loading screen?
// 	UGameplayStatics::OpenLevel(GetWorld(), "spacefootball_mainmenu");
// }

void UMyGameInstance::StartSoloGame(const FLocalPlayerContext& LPC)
{
	UMyLocalPlayer* LocalPlayer = Cast<UMyLocalPlayer>(LPC.GetLocalPlayer());
	LocalPlayer->IsMultiplayer = false;
	
	LPC.GetHUD<AMyHUDMenu>()->LoadingScreenShow
		( LOCTEXT("StartingGame...", "starting game ...")
		, [LPC] () { LPC.GetHUD<AMyHUDMenu>()->MenuSoloShow(); }
		);

	UGameplayStatics::OpenLevel(GetWorld(), "/Game/Maps/Spacefootball");
}

bool UMyGameInstance::JoinSession(ULocalPlayer* LocalPlayer, const FOnlineSessionSearchResult& SearchResult)
{
	Cast<UMyLocalPlayer>(LocalPlayer)->IsMultiplayer = true;
	FLocalPlayerContext LPC = FLocalPlayerContext(LocalPlayer);
	return GetSubsystem<UMySessionManager>()->JoinSession(LPC, SearchResult, [this, LPC] (FName SessionName, EOnJoinSessionCompleteResult::Type Result)
	{
		const std::function GotoServerList = [this, LPC] ()
		{
			AMyHUDMenu* HUDMenu = LPC.GetHUD<AMyHUDMenu>();
			if(IsValid(HUDMenu))
			{
				HUDMenu->ServerListShow();
			}
			else
			{
				UE_LOGFMT(LogNet, Warning, "{}: OnJoinSessionComplete/GotoServerList: Menu HUD invalid.", GetFName());
			}
		};

		switch(Result)
		{
		using namespace EOnJoinSessionCompleteResult;
		case SessionIsFull:
			LPC.GetHUD<AMyHUDMenu>()->MessageShow(LOCTEXT("SessionIsFull", "error: session is full"), GotoServerList);
			break;
		case SessionDoesNotExist:
			LPC.GetHUD<AMyHUDMenu>()->MessageShow(LOCTEXT("SessionIsDoesNotExist", "error: session does not exist"), GotoServerList);
			break;
		case CouldNotRetrieveAddress:
			LPC.GetHUD<AMyHUDMenu>()->MessageShow(LOCTEXT("SessionCouldNotRetrieveAddress", "error: could not retrieve address"), GotoServerList);
			break;
		case AlreadyInSession:
			LPC.GetHUD<AMyHUDMenu>()->MessageShow(LOCTEXT("SessionAlreadyInSession", "error: already in session"), GotoServerList);
			break;
		case UnknownError:
			LPC.GetHUD<AMyHUDMenu>()->MessageShow(LOCTEXT("SessionUnknownError", "error: unknown error"), GotoServerList);
			break;
		case Success:
			if(!ClientTravelToSession(LPC.GetLocalPlayer()->GetControllerId(), NAME_GameSession))
			{
				LPC.GetHUD<AMyHUDMenu>()->MessageShow(LOCTEXT("ClientTravelToSessionFailed", "error: travel to session failed"), GotoServerList);
			}
			break;
		default: ;
		}
	});
}

void UMyGameInstance::MulticastRPC_LeaveSession_Implementation()
{
	GetSubsystem<UMySessionManager>()->LeaveSession();
}

void UMyGameInstance::QuitGame(const FLocalPlayerContext& LPC)
{
	if(Cast<UMyLocalPlayer>(LPC.GetLocalPlayer())->IsMultiplayer)
	{
		GetSubsystem<UMySessionManager>()->LeaveSession([] (FName, bool) {});
	}
	UKismetSystemLibrary::QuitGame
		( GetWorld()
		, LPC.GetPlayerController()
		, EQuitPreference::Quit
		, false
		);
}

int32 UMyGameInstance::AddLocalPlayer(ULocalPlayer* NewPlayer, FPlatformUserId UserId)
{
	int32 InsertIndex = Super::AddLocalPlayer(NewPlayer, UserId);

	/*
	 
	UMyLocalPlayer* LocalPlayer = Cast<UMyLocalPlayer>(NewPlayer);
	
	// for the identity interface, SubsystemName == "NULL" doesn't make sense, I believe, as there
	// is nor identity provider in the NULL/LAN online subystem
	const IOnlineIdentityPtr OSSIdentity = Online::GetIdentityInterfaceChecked("EOS");
	
	// TODO save unique net id in some config and check if still logged in
	// TODO: creating a unique net id isn't recommended
	//       unique net ids have a type that must match the online subsystem (e.g. EOS, Steam, NULL)
	//       otherwise GameMode::PreLogin will fail
	LocalPlayer->SetCachedUniqueNetId(FUniqueNetIdRepl(OSSIdentity->CreateUniquePlayerId(FGuid::NewGuid().ToString())));
	// LocalPlayer->IsLoggedIn = OSSIdentity->GetLoginStatus(LocalPlayer->GetCachedUniqueNetId()) == ELoginStatus::LoggedIn;
	*/
	
	return InsertIndex;
}

void UMyGameInstance::Init()
{
	Super::Init();
	FCoreUObjectDelegates::PreLoadMap.AddLambda([] (const FString& MapName)
	{
		LoadingScreenModule().StartIngameLoadingScreen(true, 3.);
		//StartIngameLoadingScreen(true, 3.);
	});
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddLambda([] (UWorld* InLoadedWorld)
	{
		LoadingScreenModule().StopIngameLoadingScreen();
	});
}


#undef LOCTEXT_NAMESPACE
