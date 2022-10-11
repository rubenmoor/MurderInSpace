// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/UW_MenuInGame.h"
#include "HUD/MyCommonButton.h"
#include "Modes/MyGameInstance.h"

void UUW_MenuInGame::NativeConstruct()
{
	Super::NativeConstruct();

	UMyGameInstance* GI = GetGameInstance<UMyGameInstance>();
	BtnResume->OnClicked().AddLambda([GI] () { GI->GotoInGame(); });
	BtnLeave->OnClicked().AddLambda([GI] () { GI->GotoInMenuMain(); });
	BtnQuit->OnClicked().AddLambda([GI] () { GI->QuitGame(); });
}
