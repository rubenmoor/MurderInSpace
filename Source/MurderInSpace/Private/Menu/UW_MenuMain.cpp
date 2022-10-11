// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu/UW_MenuMain.h"

#include "HUD/MyCommonButton.h"
#include "HUD/MyHUDMenu.h"
#include "Modes/MyGameInstance.h"

void UUW_MenuMain::NativeConstruct()
{
	Super::NativeConstruct();

	BtnSinglePlayer->OnClicked().AddLambda([this]
	{
		GetOwningPlayer()->GetHUD<AMyHUDMenu>()->MenuSoloShow();
	});
	BtnLocalMultiplayer->OnClicked().AddLambda([this]
	{
		const TObjectPtr<UMyGameInstance> GI = GetGameInstance<UMyGameInstance>();
		GI->SessionConfig.bEnableLAN = true;
		GetOwningPlayer()->GetHUD<AMyHUDMenu>()->MenuMultiplayerShow();
	});
	BtnOnlineMultiplayer->OnClicked().AddLambda([this] ()
	{
		BtnLogin->SetVisibility(ESlateVisibility::Visible);
		BtnLogin->SetFocus();
	});
	BtnLogin->OnClicked().AddLambda([this] ()
	{
		// TODO: call to online subsystem for login
		// on return (callback?):
		/*
		GI->SessionConfig.bEnableLAN = false;
		GetOwningPlayer()->GetHUD<AMyHUDMenu>()->MenuMultiplayerShow();
		*/
	});
	// TODO: BtnLogin: on focus lost: hide
	BtnQuit->OnClicked().AddLambda([this] ()
	{
		GetGameInstance<UMyGameInstance>()->QuitGame();
	});
}
