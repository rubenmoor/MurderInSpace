// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/GameMode_Menu.h"

#include "Modes/MyLocalPlayer.h"

void AGameMode_Menu::BeginPlay()
{
	Super::BeginPlay();
	const FInputModeUIOnly InputModeUIOnly;
	const FInputModeGameAndUI InputModeGameAndUI;
	GetWorld()->GetFirstPlayerController()->SetInputMode(InputModeGameAndUI);
}
