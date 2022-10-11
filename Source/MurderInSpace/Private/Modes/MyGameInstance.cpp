// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MyGameInstance.h"

#include "HUD/MyHUD.h"
#include "HUD/MyHUDMenu.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Lib/FunctionLib.h"
#include "Menu/UW_ServerList.h"
#include "Modes/MyGISubsystem.h"

#define LOCTEXT_NAMESPACE "Menu"

UMyGameInstance::UMyGameInstance()
{
	// I want random numbers to be the same across multiplayer clients
	// So the game instance can't create a seed --> maybe generate rnd elsewhere entirely
}

void UMyGameInstance::HostGame()
{
	const TObjectPtr<UMyGISubsystem> GISub = GetSubsystem<UMyGISubsystem>();
	// ReSharper disable once CppTooWideScope
	const bool bSuccess = GISub->CreateSession
		( SessionConfig
		, [this] (FName SessionName, bool bSuccess)
		{
			switch(InstanceState)
			{
			case EInstanceState::WaitingForSessionCreate:
				if(bSuccess)
				{
					bIsMultiplayer = true;
					GotoInGame();
				}
				else
				{
					const TObjectPtr<AMyHUDMenu> HUDMenu = GetPrimaryPlayerController()->GetHUD<AMyHUDMenu>();
					HUDMenu->MessageShow(LOCTEXT
						( "CreateSessionOnlineSubsystemFailure", "the online subsystem returned with failure")
						, [this] () { GotoInMenuMain(); }
						);
					UE_LOG(LogNet, Error, TEXT("%s: create session call returned with failure"), *GetFullName())
				}
				break;
			default:
				ErrorWrongState
					( this, UEnum::GetValueAsString(EInstanceState::WaitingForSessionCreate) );
			}
		});
	
	const TObjectPtr<AMyHUDMenu> HUDMenu = GetPrimaryPlayerController()->GetHUD<AMyHUDMenu>();
	if(bSuccess)
	{
		GotoWaitingForSessionCreate();
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

void UMyGameInstance::GotoInMenuMain()
{
	UE_LOG(LogSlate, Warning, TEXT("Debug: GotoInMenuMain"))
	switch(InstanceState)
	{
	case EInstanceState::WaitingForSessionCreate:
		// try to destroy the session, in case it has been created; but fail silently otherwise
		GetSubsystem<UMyGISubsystem>()->DestroySession([] (FName, bool) {});
		GetPrimaryPlayerController()->GetHUD<AMyHUDMenu>()->MenuMainShow();
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

void UMyGameInstance::GotoInGame()
{
	const FInputModeGameOnly InputModeGameOnly;
	switch(InstanceState)
	{
	case EInstanceState::InGame:
		InGameMenuHide();
		break;
	case EInstanceState::WaitingForSessionCreate:
		bIsMultiplayer = true;
		// fall through
	case EInstanceState::InMainMenu:
		UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("spacefootball")));
		bShowInGameMenu = false;
		break;
	default:
		ErrorWrongState
			( this
			, UEnum::GetValueAsString(EInstanceState::InMainMenu)
			+ UEnum::GetValueAsString(EInstanceState::WaitingForSessionCreate)
			);
		return;
	}
	InstanceState = EInstanceState::InGame;
}

void UMyGameInstance::InGameMenuShow()
{
	if(!bShowInGameMenu)
	{
		GetPrimaryPlayerController()->GetHUD<AMyHUD>()->InGameMenuShow();
	}
	bShowInGameMenu = true;
}

void UMyGameInstance::InGameMenuHide()
{
	if(bShowInGameMenu)
	{
		GetPrimaryPlayerController()->GetHUD<AMyHUD>()->InGameMenuHide();
	}
	bShowInGameMenu = false;
}

void UMyGameInstance::GotoWaitingForSessionCreate()
{
	switch(InstanceState)
	{
	case EInstanceState::WaitingForSessionCreate:
	case EInstanceState::InMainMenu:
		const TObjectPtr<AMyHUDMenu> HUDMenu = GetPrimaryPlayerController()->GetHUD<AMyHUDMenu>();
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
			( this, UEnum::GetValueAsString(EInstanceState::WaitingForSessionCreate));
		return;
	}
	InstanceState = EInstanceState::WaitingForSessionCreate;
}

void UMyGameInstance::JoinGame()
{
}

void UMyGameInstance::ServerListRefresh()
{
	// TODO: why isn't the unique net id valid?
	const TObjectPtr<AMyHUDMenu> HUDMenu = GetPrimaryPlayerController(false)->GetHUD<AMyHUDMenu>();
	HUDMenu->WidgetServerList->SetStatusMessage(LOCTEXT("SearchingSessions", "Searching for sessions ..."));
	HUDMenu->WidgetServerList->SetBtnRefreshEnabled(false);
	GetSubsystem<UMyGISubsystem>()->FindSessions([this, HUDMenu] (bool bSuccess)
	{
		HUDMenu->WidgetServerList->SetBtnRefreshEnabled(true);
		if(bSuccess)
		{
			HUDMenu->ServerListUpdate();
		}
		else
		{
			HUDMenu->MessageShow
				( LOCTEXT("ErrorSessionSearch", "Error when trying to search for sessions")
				, [HUDMenu] () { HUDMenu->MenuMainShow(); }
				);
		}
	});
}

void UMyGameInstance::QuitGame()
{
	if(bIsMultiplayer)
	{
		GetSubsystem<UMyGISubsystem>()->DestroySession([] (FName, bool) {});
	}
	UKismetSystemLibrary::QuitGame
		( GetWorld()
		, GetPrimaryPlayerController()
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
