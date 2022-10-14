// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu/UW_ServerList.h"
#include "HUD/MyCommonButton.h"
#include "HUD/MyHUDMenu.h"
#include "Modes/MyGameInstance.h"
#include "Modes/MyGISubsystem.h"

void UUW_ServerList::DeselectAllBut(int32 Index)
{
	TArray<UWidget*> Rows = ScrollServers->GetAllChildren();
	//TArray<UUW_ServerRow*> Rows = ScrollServers->GetAllChildren();
	//auto Rows = ScrollServers->GetAllChildren();
	for(int32 i = 0; i < Rows.Num(); i++)
	{
		if(i == Index)
		{
			continue;
		}
		Cast<UUW_ServerRow>(Rows[i])->SetIsSelected(false);
	}
}

void UUW_ServerList::NativeConstruct()
{
	Super::NativeConstruct();

	BtnBack->OnClicked().AddLambda([this] ()
	{
		GetPlayerContext().GetHUD<AMyHUDMenu>()->MenuMultiplayerShow();
	});
	BtnRefresh->OnClicked().AddLambda([this] ()
	{
		GetPlayerContext().GetHUD<AMyHUDMenu>()->ServerListRefresh();
	});
	BtnJoin->OnClicked().AddLambda([this] ()
	{
		Cast<UMyGameInstance>(GetGameInstance())->JoinSession
			( GetOwningLocalPlayer()
			, GetGameInstance()->GetSubsystem<UMyGISubsystem>()->GetSearchResult()[SelectedIndex]
			);
	});
}
