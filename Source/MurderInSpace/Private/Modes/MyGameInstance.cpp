// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MyGameInstance.h"

#include "HUD/MyHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Modes/MyGISubsystem.h"

UMyGameInstance::UMyGameInstance()
{
	// I want random numbers to be the same acros multiplayer clients
	// So the game instance can't create a seed --> maybe generate rnd elsewhere entirely
	// Random.GenerateNewSeed();
}

void UMyGameInstance::HostGame()
{
	// try create session
	// TODO: configure session
	const TObjectPtr<UMyGISubsystem> GISub = GetWorld()->GetGameInstance()->GetSubsystem<UMyGISubsystem>();
	GISub->CreateSession(4, true);
	// register handler
	// success: open level
	UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("spacefootball")));
	// failure: back to main menu with error message
}

void UMyGameInstance::GotoInMenuMain()
{
	UE_LOG(LogSlate, Warning, TEXT("Debug: GotoInMenuMain"))
	switch(InstanceState)
	{
	case EInstanceState::InGame:
		// TODO: destroy session
		// TODO: switch level to menu
		UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("spacefootball_mainmenu")));
		break;
	default:
		ErrorWrongState
			(  UEnum::GetValueAsString(EInstanceState::InGame)
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
	case EInstanceState::InGameMenu:
		const TObjectPtr<APlayerController> PC = GetPrimaryPlayerController();
		PC->GetHUD<AMyHUD>()->InGameMenuHide();
		//PC->SetInputMode(InputModeGameOnly);
		break;
	case EInstanceState::InMainMenu:
		UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("spacefootball")));
		break;
	default:
		ErrorWrongState
			( UEnum::GetValueAsString(EInstanceState::InMainMenu)
			+ UEnum::GetValueAsString(EInstanceState::InGameMenu)
			);
		return;
	}
	InstanceState = EInstanceState::InGame;
}

void UMyGameInstance::GotoInGameMenu()
{
	const FInputModeUIOnly InputModeUIOnly;
	switch (InstanceState)
	{
	case EInstanceState::InGame:
		const TObjectPtr<APlayerController> PC = GetPrimaryPlayerController();
		PC->GetHUD<AMyHUD>()->InGameMenuShow();
		// TODO: seems elegant, but disables my ability to go back to InGame using the same action
		//PC->SetInputMode(InputModeUIOnly);
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
