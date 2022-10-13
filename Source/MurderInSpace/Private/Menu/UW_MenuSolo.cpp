// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu/UW_MenuSolo.h"

#include "HUD/MyCommonButton.h"
#include "HUD/MyHUDMenu.h"
#include "Modes/MyGameInstance.h"

void UUW_MenuSolo::NativeConstruct()
{
	Super::NativeConstruct();

	const TObjectPtr<UMyGameInstance> GI = GetGameInstance<UMyGameInstance>();
	
	BtnStartNew->OnClicked().AddLambda([this, GI] () { GI->StartSoloGame(GetOwningPlayer()); });
	BtnBack->OnClicked().AddLambda([this] () { GetOwningPlayer()->GetHUD<AMyHUDMenu>()->MenuMainShow(); } );
}
