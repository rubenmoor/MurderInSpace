// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu/UW_ServerList.h"
#include "HUD/MyCommonButton.h"
#include "HUD/MyHUDMenu.h"
#include "Modes/MyGameInstance.h"
#include "Modes/MySessionManager.h"

void UUW_ServerList::DeselectAllBut(int32 Index)
{
	SelectedIndex = Index;
	
	for(int32 i = 0; i < ScrollServers->GetChildrenCount(); i++)
	{
		if(i != Index)
		{
			Cast<UUW_ServerRow>(ScrollServers->GetChildAt(i))->SetSelectedInternal(false, true, false);
		}
	}
}

void UUW_ServerList::SelectFirst()
{
	Cast<UUW_ServerRow>(ScrollServers->GetChildAt(0))->SetIsSelected(true);
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
			, GetGameInstance()->GetSubsystem<UMySessionManager>()->GetSearchResult()[SelectedIndex]
			);
	});
}
