// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MyGameInstance.h"

#include "Online.h"
#include "HUD/MyHUDMenu.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Modes/MyGISubsystem.h"
#include "Modes/MyLocalPlayer.h"


#define LOCTEXT_NAMESPACE "Menu"

UMyGameInstance::UMyGameInstance()
{
	// I want random numbers to be the same across multiplayer clients
	// So the game instance can't create a seed --> maybe generate rnd elsewhere entirely
}

void UMyGameInstance::HostGame(const FLocalPlayerContext& LPC)
{
	UMyGISubsystem* GISub = GetSubsystem<UMyGISubsystem>();
	AMyHUDMenu* HUDMenu = LPC.GetHUD<AMyHUDMenu>();
	
	// ReSharper disable once CppTooWideScope
	const bool bSuccess = GISub->CreateSession
		( LPC
		, SessionConfig
		, [this, LPC, HUDMenu] (FName SessionName, bool bSuccess)
		{
			if(bSuccess)
			{
				Cast<UMyLocalPlayer>(LPC.GetLocalPlayer())->CurrentLevel = ECurrentLevel::SpaceFootball;
				UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("spacefootball")));
			}
			else
			{
				HUDMenu->MessageShow(LOCTEXT
					( "CreateSessionOnlineSubsystemFailure", "the online subsystem returned with failure")
					, [&LPC] () { LPC.GetHUD<AMyHUDMenu>()->MenuMainShow(); }
					);
				UE_LOG(LogNet, Error, TEXT("%s: create session call returned with failure"), *GetFullName())
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
		UE_LOG(LogNet, Error, TEXT("%s: couldn't create session"), *GetFullName())
	}
}

void UMyGameInstance::LeaveGame(const FLocalPlayerContext& LPC)
{
	UMyLocalPlayer* LocalPlayer = Cast<UMyLocalPlayer>(LPC.GetLocalPlayer());
	if(LocalPlayer->IsMultiplayer)
	{
		GetSubsystem<UMyGISubsystem>()->DestroySession([] (FName, bool) {} );
	}
	// TODO: loading screen?
	LocalPlayer->CurrentLevel = ECurrentLevel::MainMenu;
	UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("spacefootball_mainmenu")));
}

void UMyGameInstance::StartSoloGame(const FLocalPlayerContext& LPC)
{
	UMyLocalPlayer* LocalPlayer = Cast<UMyLocalPlayer>(LPC.GetLocalPlayer());
	LocalPlayer->IsMultiplayer = false;
	LocalPlayer->CurrentLevel = ECurrentLevel::SpaceFootball;
	
	// TODO: howto show loading screen?
	// HUDMenu->LoadingScreenShow
	// 	( LOCTEXT("StartingGame...", "starting game ...")
	// 	, [HUDMenu] () { HUDMenu->MenuSoloShow(); }
	// 	);

	UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("spacefootball")));
}

void UMyGameInstance::JoinGame(const FLocalPlayerContext& LPC)
{
	Cast<UMyLocalPlayer>(LPC.GetLocalPlayer())->IsMultiplayer = true;
	// TODO
}

void UMyGameInstance::QuitGame(const FLocalPlayerContext& LPC)
{
	if(Cast<UMyLocalPlayer>(LPC.GetLocalPlayer())->IsMultiplayer)
	{
		GetSubsystem<UMyGISubsystem>()->DestroySession([] (FName, bool) {});
	}
	UKismetSystemLibrary::QuitGame
		( GetWorld()
		, LPC.GetPlayerController()
		, EQuitPreference::Quit
		, false
		);
}

int32 UMyGameInstance::AddLocalPlayer(ULocalPlayer* NewPlayer, int32 ControllerId)
{
	int32 InsertIndex = Super::AddLocalPlayer(NewPlayer, ControllerId);
	UMyLocalPlayer* LocalPlayer = Cast<UMyLocalPlayer>(NewPlayer);
	LocalPlayer->CurrentLevel = ECurrentLevel::MainMenu;
	LocalPlayer->IsMultiplayer = false;
	LocalPlayer->ShowInGameMenu = false;

	// for the identity interface, SubsystemName == "NULL" doesn't make sense, I believe, as there
	// is nor identity provider in the NULL/LAN online subystem
	const IOnlineIdentityPtr OSSIdentity = Online::GetIdentityInterfaceChecked(FName(TEXT("EOS")));
	
	// TODO save unique net id in some config and check if still logged in
	LocalPlayer->SetCachedUniqueNetId(FUniqueNetIdRepl(OSSIdentity->CreateUniquePlayerId(FGuid::NewGuid().ToString())));
	// LocalPlayer->IsLoggedIn = OSSIdentity->GetLoginStatus(LocalPlayer->GetCachedUniqueNetId()) == ELoginStatus::LoggedIn;
	LocalPlayer->IsLoggedIn = false;
	return InsertIndex;
}

#undef LOCTEXT_NAMESPACE
