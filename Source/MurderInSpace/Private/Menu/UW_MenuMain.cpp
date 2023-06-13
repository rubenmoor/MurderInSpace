// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu/UW_MenuMain.h"

#include "HUD/MyCommonButton.h"
#include "HUD/MyHUDMenu.h"
#include "Modes/MyGameInstance.h"
#include "Modes/MySessionManager.h"
#include "Modes/MyLocalPlayer.h"

#define LOCTEXT_NAMESPACE "Menu"

void UUW_MenuMain::NativeConstruct()
{
	Super::NativeConstruct();

	BtnSinglePlayer->OnClicked().AddLambda([this]
	{
		GetPlayerContext().GetHUD<AMyHUDMenu>()->MenuSoloShow();
	});
	BtnLocalMultiplayer->OnClicked().AddLambda([this]
	{
		UMyGameInstance* GI = GetGameInstance<UMyGameInstance>();
		GI->SessionConfig.bEnableLAN = true;
		GetPlayerContext().GetHUD<AMyHUDMenu>()->MenuMultiplayerShow();
	});
	BtnOnlineMultiplayer->OnClicked().AddLambda([this] ()
	{
		UMyGameInstance* GI = GetGameInstance<UMyGameInstance>();
		if(Cast<UMyLocalPlayer>(GetOwningLocalPlayer())->IsLoggedIn)
		{
			GI->SessionConfig.bEnableLAN = false;
			GetPlayerContext().GetHUD<AMyHUDMenu>()->MenuMultiplayerShow();
		}
		else
		{
			BtnLogin->SetVisibility(ESlateVisibility::Visible);
			BtnLogin->SetFocus();
		}
	});
	BtnLogin->OnClicked().AddLambda([this] ()
	{
		const TObjectPtr<UMyGameInstance> GI = GetGameInstance<UMyGameInstance>();
		GI->GetSubsystem<UMySessionManager>()->ShowLoginScreen(GetPlayerContext());
		BtnLogin->SetVisibility(ESlateVisibility::Collapsed);
		// AMyHUDMenu* HUDMenu = GetOwningPlayer()->GetHUD<AMyHUDMenu>();
		// HUDMenu->LoadingScreenShow
		// 	(LOCTEXT("WaitingLogin...", "waiting for login to complete ...")
		// 	, [HUDMenu] () { HUDMenu->MenuMainShow(); }
		// 	);
	});
	// TODO: BtnLogin: on focus lost: hide
	BtnSettings->OnClicked().AddLambda([this] ()
	{
		GetPlayerContext().GetHUD<AMyHUDBase>()->SettingsShow();
	});
	
	BtnQuit->OnClicked().AddLambda([this] ()
	{
		GetGameInstance<UMyGameInstance>()->QuitGame(GetPlayerContext());
	});
}
#undef LOCTEXT_NAMESPACE
