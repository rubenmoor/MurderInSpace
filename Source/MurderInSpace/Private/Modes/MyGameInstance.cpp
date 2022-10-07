// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MyGameInstance.h"

#include "HUD/MyHUD.h"
#include "HUD/MyHUDMenu.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Modes/MyGISubsystem.h"

#define LOCTEXT_NAMESPACE "Menu"

UMyGameInstance::UMyGameInstance()
{
	// I want random numbers to be the same acros multiplayer clients
	// So the game instance can't create a seed --> maybe generate rnd elsewhere entirely
	// Random.GenerateNewSeed();
}

void UMyGameInstance::HostGame()
{
	// TODO: configure session
	const TObjectPtr<UMyGISubsystem> GISub = GetSubsystem<UMyGISubsystem>();
	// ReSharper disable once CppTooWideScope
	const ESIResult Result = GISub->CreateSession
		(4
		, true
		, [this] (FName SessionName, bool bSuccess)
		{
			switch(InstanceState)
			{
			case EInstanceState::WaitingForSessionCreate:
				if(bSuccess)
				{
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
	switch(Result)
	{
	case ESIResult::Failure:
		HUDMenu->MessageShow(LOCTEXT
			( "CreateSessionReturnedFailure", "call to create session returned failure" )
			, [HUDMenu] () { HUDMenu->MainMenuShow(); }
			);
		UE_LOG(LogNet, Error, TEXT("%s: couldn't create session"), *GetFullName())
		break;
	case ESIResult::NoSessionInterface:
		HUDMenu->MessageShow(LOCTEXT
			( "CouldntGetSessionInterface", "couldn't get session interface" )
			, [HUDMenu] () { HUDMenu->MainMenuShow(); }
			);
		UE_LOG(LogNet, Error, TEXT("%s: couldn't get session interface"), *GetFullName())
		break;
	case ESIResult::Success:
		GotoWaitingForSessionCreate();
	}
}

void UMyGameInstance::GotoInMenuMain()
{
	UE_LOG(LogSlate, Warning, TEXT("Debug: GotoInMenuMain"))
	switch(InstanceState)
	{
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
		// TODO: seems elegant, but disables my ability to go back to InGame using the same action
		//PC->SetInputMode(InputModeUIOnly);
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
	case EInstanceState::InMainMenu:
		const TObjectPtr<APlayerController> PC = GetPrimaryPlayerController();
		PC->GetHUD<AMyHUDMenu>()->LoadingScreenShow(LOCTEXT("CreatingSession...", "creating session ..."));
		break;
	default:
		ErrorWrongState
			( this, UEnum::GetValueAsString(EInstanceState::InMainMenu));
		return;
	}
	InstanceState = EInstanceState::WaitingForSessionCreate;
}

void UMyGameInstance::JoinGame()
{
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
