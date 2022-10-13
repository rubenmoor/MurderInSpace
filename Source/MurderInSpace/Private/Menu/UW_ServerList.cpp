// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu/UW_ServerList.h"
#include "HUD/MyCommonButton.h"
#include "HUD/MyHUDMenu.h"
#include "Modes/MyGameInstance.h"

void UUW_ServerList::NativeConstruct()
{
	Super::NativeConstruct();

	BtnBack->OnClicked().AddLambda([this] ()
	{
		GetOwningPlayer()->GetHUD<AMyHUDMenu>()->MenuMultiplayerShow();
	});
	BtnRefresh->OnClicked().AddLambda([this] ()
	{
		GetOwningPlayer()->GetHUD<AMyHUDMenu>()->ServerListRefresh();
	});
	// TODO: server row on clicked -> GI -> join game { join session }
}
