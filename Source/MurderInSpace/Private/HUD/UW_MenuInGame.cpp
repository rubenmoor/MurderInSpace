// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/UW_MenuInGame.h"
#include "HUD/MyCommonButton.h"
#include "HUD/MyHUD.h"
#include "Modes/MyGameInstance.h"

void UUW_MenuInGame::NativeConstruct()
{
	Super::NativeConstruct();

	BtnResume->OnClicked().AddLambda([this] ()
	{
		GetPlayerContext().GetHUD<AMyHUD>()->InGameMenuHide();
	});
	BtnLeave->OnClicked().AddLambda([this] ()
	{
		//GetGameInstance<UMyGameInstance>()->LeaveGame(GetPlayerContext());
		GetGameInstance()->ReturnToMainMenu();
	});
	BtnQuit->OnClicked().AddLambda([this] ()
	{
		GetGameInstance<UMyGameInstance>()->QuitGame(GetPlayerContext());
	});
}
