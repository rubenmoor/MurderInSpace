// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/UW_MenuInGame.h"
#include "HUD/MyCommonButton.h"
#include "Modes/MyGameInstance.h"

void UUW_MenuInGame::NativeConstruct()
{
	Super::NativeConstruct();

	UMyGameInstance* GI = GetGameInstance<UMyGameInstance>();
	BtnResume->OnClicked().AddLambda([this, GI] () { GI->GotoInGame(GetOwningPlayer()); });
	BtnLeave->OnClicked().AddLambda([this, GI] () { GI->GotoInMenuMain(GetOwningPlayer()); });
	BtnQuit->OnClicked().AddLambda([this, GI] () { GI->QuitGame(GetOwningPlayer()); });
}
