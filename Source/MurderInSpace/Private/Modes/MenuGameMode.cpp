// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MenuGameMode.h"

#include "Modes/MyGameInstance.h"

void AMenuGameMode::BeginPlay()
{
	Super::BeginPlay();
	const FInputModeUIOnly InputModeUIOnly;
	const FInputModeGameAndUI InputModeGameAndUI;
	GetWorld()->GetFirstPlayerController()->SetInputMode(InputModeGameAndUI);
}
