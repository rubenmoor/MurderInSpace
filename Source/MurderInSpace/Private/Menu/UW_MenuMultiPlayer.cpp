// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu/UW_MenuMultiPlayer.h"

#include "HUD/MyCommonButton.h"
#include "HUD/MyHUDMenu.h"

void UUW_MenuMultiPlayer::NativeConstruct()
{
	Super::NativeConstruct();
	
	BtnCreateNew->OnClicked().AddLambda([this] () { GetPlayerContext().GetHUD<AMyHUDMenu>()->HostGameShow(); });
	BtnJoin->OnClicked().AddLambda([this] () { GetPlayerContext().GetHUD<AMyHUDMenu>()->ServerListShow(); });
	BtnBack->OnClicked().AddLambda([this] () { GetPlayerContext().GetHUD<AMyHUDMenu>()->MenuMainShow(); } );
}
