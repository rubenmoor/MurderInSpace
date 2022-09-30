// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"

#include "PawnInSpace.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UObject/UObjectIterator.h"

UMyGameInstance::UMyGameInstance()
{
	Random.GenerateNewSeed();
}

void UMyGameInstance::HostGame()
{
}

void UMyGameInstance::ShowServers()
{
}

void UMyGameInstance::ShowMainMenu()
{
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
