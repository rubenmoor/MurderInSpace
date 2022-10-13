// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MyGameInstance.h"

#include "HUD/MyHUD.h"
#include "HUD/MyHUDMenu.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Modes/MyGISubsystem.h"
#include "Modes/MyPlayerState.h"

#define LOCTEXT_NAMESPACE "Menu"

UMyGameInstance::UMyGameInstance()
{
	// I want random numbers to be the same across multiplayer clients
	// So the game instance can't create a seed --> maybe generate rnd elsewhere entirely
}

// get the local user index given a unique net id
int32 UMyGameInstance::GetLocalPlayerIndex(FUniqueNetIdRepl UNI)
{
	int32 FoundIndex = -1;
	for(int32 i = 0; i < LocalPlayers.Num(); i++)
	{
		if(LocalPlayers[i]->GetCachedUniqueNetId() == UNI)
		{
			FoundIndex = i;
			break;
		}
	}
	if(FoundIndex == -1)
	{
		UE_LOG
			( LogNet
			, Error
			, TEXT("%s: Couldn't find local player index for unique net id %s")
			, *GetFullName()
			, *UNI.ToString()
			)
	}
	return FoundIndex;
}

void UMyGameInstance::HostGame(const APlayerController* PC)
{
	UMyGISubsystem* GISub = GetSubsystem<UMyGISubsystem>();
	AMyHUDMenu* HUDMenu = PC->GetHUD<AMyHUDMenu>();
	
	// ReSharper disable once CppTooWideScope
	const bool bSuccess = GISub->CreateSession
		( PC->GetPlayerState<AMyPlayerState>()->GetUniqueId()
		, SessionConfig
		, [this, PC, HUDMenu] (FName SessionName, bool bSuccess)
		{
			switch(InstanceState)
			{
			case EInstanceState::WaitingForStart:
			// looks like OnCreateSessionComplete fire immediately and we are still in main menu
			case EInstanceState::InMainMenu:
				if(bSuccess)
				{
					GotoInGame(PC);
				}
				else
				{
					HUDMenu->MessageShow(LOCTEXT
						( "CreateSessionOnlineSubsystemFailure", "the online subsystem returned with failure")
						, [this, PC] () { GotoInMenuMain(PC); }
						);
					UE_LOG(LogNet, Error, TEXT("%s: create session call returned with failure"), *GetFullName())
				}
				break;
			default:
				ErrorWrongState
					( this, UEnum::GetValueAsString(EInstanceState::WaitingForStart) );
			}
		});
	
	if(bSuccess)
	{
		bIsMultiplayer = true;
		GotoWaitingForStart(PC);
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

void UMyGameInstance::StartSoloGame(const APlayerController* PC)
{
	bIsMultiplayer = false;
	GotoInGame(PC);
}

void UMyGameInstance::GotoInMenuMain(const APlayerController* PC)
{
	UE_LOG(LogSlate, Warning, TEXT("Debug: GotoInMenuMain"))
	switch(InstanceState)
	{
	case EInstanceState::WaitingForStart:
		// try to destroy the session, in case it has been created; but fail silently otherwise
		GetSubsystem<UMyGISubsystem>()->DestroySession([] (FName, bool) {});
		PC->GetHUD<AMyHUDMenu>()->MenuMainShow();
	case EInstanceState::InGame:
		if(bIsMultiplayer)
		{
			GetSubsystem<UMyGISubsystem>()->DestroySession([this] (FName SessionName, bool bSuccess)
			{
				UE_LOG
					( LogNet
					, Display
					, TEXT("%s: session %s destroyed %s")
					, *GetFullName()
					, *SessionName.ToString()
					// ReSharper disable once CppPrintfBadFormat
					, bSuccess ? TEXT("successfully") : TEXT("failure")
					)
			});
		}
		UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("spacefootball_mainmenu")));
		break;
	default:
		ErrorWrongState
			(  this, UEnum::GetValueAsString(EInstanceState::InGame)
			);
		return;
	}
	InstanceState = EInstanceState::InMainMenu;
}

void UMyGameInstance::GotoInGame(const APlayerController* PC)
{
	const FInputModeGameOnly InputModeGameOnly;
	switch(InstanceState)
	{
	case EInstanceState::InGame:
		InGameMenuHide(PC);
		break;
	case EInstanceState::WaitingForStart:
	case EInstanceState::InMainMenu:
		UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("spacefootball")));
		bShowInGameMenu = false;
		break;
	default:
		ErrorWrongState
			( this
			, UEnum::GetValueAsString(EInstanceState::InMainMenu)
			+ UEnum::GetValueAsString(EInstanceState::WaitingForStart)
			);
		return;
	}
	InstanceState = EInstanceState::InGame;
}

void UMyGameInstance::InGameMenuShow(const APlayerController* PC)
{
	if(!bShowInGameMenu)
	{
		PC->GetHUD<AMyHUD>()->InGameMenuShow();
	}
	bShowInGameMenu = true;
}

void UMyGameInstance::InGameMenuHide(const APlayerController* PC)
{
	if(bShowInGameMenu)
	{
		PC->GetHUD<AMyHUD>()->InGameMenuHide();
	}
	bShowInGameMenu = false;
}

void UMyGameInstance::GotoWaitingForStart(const APlayerController* PC)
{
	AMyHUDMenu* HUDMenu = PC->GetHUD<AMyHUDMenu>();
	switch(InstanceState)
	{
	case EInstanceState::WaitingForStart:
	case EInstanceState::InMainMenu:
		if(bIsMultiplayer)
		{
			HUDMenu->LoadingScreenShow
				( SessionConfig.bEnableLAN
					? LOCTEXT("CreatingLANSession...", "creating LAN session ...")
					: LOCTEXT("CreatingOnlineSession", "creating online session ...")
				, [HUDMenu] () { HUDMenu->MenuMultiplayerShow(); }
				);
		}
		else
		{
			HUDMenu->LoadingScreenShow
				( LOCTEXT("StartingGame...", "starting game ...")
				, [HUDMenu] () { HUDMenu->MenuSoloShow(); }
				);
		}
		break;
	default:
		ErrorWrongState
			( this, UEnum::GetValueAsString(EInstanceState::WaitingForStart));
		return;
	}
	InstanceState = EInstanceState::WaitingForStart;
}

void UMyGameInstance::JoinGame()
{
	bIsMultiplayer = true;
	// TODO
}

void UMyGameInstance::QuitGame(APlayerController* PC)
{
	if(bIsMultiplayer)
	{
		GetSubsystem<UMyGISubsystem>()->DestroySession([] (FName, bool) {});
	}
	UKismetSystemLibrary::QuitGame
		( GetWorld()
		, PC
		, EQuitPreference::Quit
		, false
		);
}

void UMyGameInstance::ErrorWrongState(const UObject* Object, const FString& InStatesExpected)
{
	UE_LOG
		( LogPlayerController
		, Error
		, TEXT("%s: expected state(s): %s, actual state: %s")
		, *Object->GetFullName()
		, *InStatesExpected
		, *UEnum::GetValueAsString(InstanceState)
		)
}

#undef LOCTEXT_NAMESPACE
