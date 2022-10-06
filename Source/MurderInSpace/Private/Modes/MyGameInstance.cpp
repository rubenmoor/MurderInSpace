// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MyGameInstance.h"

#include "HUD/MyHUD.h"
#include "HUD/MyHUDMenu.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

UMyGameInstance::UMyGameInstance()
{
	// I want random numbers to be the same acros multiplayer clients
	// So the game instance can't create a seed --> maybe generate rnd elsewhere entirely
	// Random.GenerateNewSeed();
}

void UMyGameInstance::HostGame()
{
	UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("spacefootball")));
}

void UMyGameInstance::GotoInMenuServers()
{
	switch(InstanceState)
	{
	case EInstanceState::InMenuMain:
		InstanceState = EInstanceState::InMenuServers;
		GetPrimaryPlayerController()->GetHUD<AMyHUDMenu>()->ServerListShow();
		break;
	default:
		ErrorWrongState(UEnum::GetValueAsString(EInstanceState::InMenuMain));
	}
}

void UMyGameInstance::GotoInMenuMain()
{
	UE_LOG(LogSlate, Warning, TEXT("Debug: GotoInMenuMain"))
	switch(InstanceState)
	{
	case EInstanceState::InMenuServers:
		GetPrimaryPlayerController()->GetHUD<AMyHUDMenu>()->MainMenuShow();
		break;
	case EInstanceState::InGame:
		// TODO: destroy session
		// TODO: switch level to menu
		UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("spacefootball_mainmenu")));
		break;
	default:
		ErrorWrongState
			( UEnum::GetValueAsString(EInstanceState::InMenuServers)
			+ UEnum::GetValueAsString(EInstanceState::InGame)
			);
		return;
	}
	InstanceState = EInstanceState::InMenuMain;
}

void UMyGameInstance::GotoInGame()
{
	switch(InstanceState)
	{
	case EInstanceState::InGameMenu:
		GetPrimaryPlayerController()->GetHUD<AMyHUD>()->InGameMenuHide();
		break;
	case EInstanceState::InMenuMain:
	case EInstanceState::InMenuServers:
		UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("spacefootball")));
		break;
	default:
		ErrorWrongState
			( UEnum::GetValueAsString(EInstanceState::InMenuMain)
			+ UEnum::GetValueAsString(EInstanceState::InMenuServers)
			+ UEnum::GetValueAsString(EInstanceState::InGameMenu)
			);
		return;
	}
	InstanceState = EInstanceState::InGame;
}

void UMyGameInstance::GotoInGameMenu()
{
	switch (InstanceState)
	{
	case EInstanceState::InGame:
		GetPrimaryPlayerController()->GetHUD<AMyHUD>()->InGameMenuShow();
		break;
	default:
		ErrorWrongState
			( UEnum::GetValueAsString(EInstanceState::InGame)
			);
		return;
	}
	InstanceState = EInstanceState::InGameMenu;
}

void UMyGameInstance::JoinGame()
{
}

void UMyGameInstance::QuitGame()
{
	UKismetSystemLibrary::QuitGame
		( GetWorld()
		, GetPrimaryPlayerController()
		, EQuitPreference::Quit
		, false
		);
}

void UMyGameInstance::ErrorWrongState(const FString& InStatesExpected)
{
	UE_LOG
		( LogPlayerController
		, Error
		, TEXT("expected state(s): %s, actual state: %s")
		, *InStatesExpected
		, *UEnum::GetValueAsString(InstanceState)
		)
}
