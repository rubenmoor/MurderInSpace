// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu/UW_MenuMain.h"

#include "HUD/MyCommonButton.h"
#include "HUD/MyHUDMenu.h"
#include "Modes/MyGameInstance.h"
#include "Modes/MyGISubsystem.h"

#define LOCTEXT_NAMESPACE "Menu"

void UUW_MenuMain::NativeConstruct()
{
	Super::NativeConstruct();

	BtnSinglePlayer->OnClicked().AddLambda([this]
	{
		GetOwningPlayer()->GetHUD<AMyHUDMenu>()->MenuSoloShow();
	});
	BtnLocalMultiplayer->OnClicked().AddLambda([this]
	{
		UMyGameInstance* GI = GetGameInstance<UMyGameInstance>();
		GI->SessionConfig.bEnableLAN = true;
		GetOwningPlayer()->GetHUD<AMyHUDMenu>()->MenuMultiplayerShow();
	});
	BtnOnlineMultiplayer->OnClicked().AddLambda([this] ()
	{
		UMyGameInstance* GI = GetGameInstance<UMyGameInstance>();
		if(GI->bLoggedIn)
		{
			GI->SessionConfig.bEnableLAN = false;
			GetOwningPlayer()->GetHUD<AMyHUDMenu>()->MenuMultiplayerShow();
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
		GI->GetSubsystem<UMyGISubsystem>()->ShowLoginScreen();
		BtnLogin->SetVisibility(ESlateVisibility::Collapsed);
		AMyHUDMenu* HUDMenu = GetOwningPlayer()->GetHUD<AMyHUDMenu>();
		HUDMenu->LoadingScreenShow
			(LOCTEXT("WaitingLogin...", "waiting for login to complete ...")
			, [HUDMenu] () { HUDMenu->MenuMainShow(); }
			);
	});
	// TODO: BtnLogin: on focus lost: hide
	BtnQuit->OnClicked().AddLambda([this] ()
	{
		GetGameInstance<UMyGameInstance>()->QuitGame();
	});
}
# undef LOCTEXT_NAM
