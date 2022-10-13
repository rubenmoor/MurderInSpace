// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu/UW_HostGame.h"
#include "CommonTextBlock.h"
#include "Components/CheckBox.h"
#include "Components/EditableTextBox.h"
#include "GameFramework/PlayerState.h"
#include "Misc/DefaultValueHelper.h"
#include "Modes/MyGameInstance.h"
#include "HUD/MyCommonButton.h"
#include "HUD/MyHUDMenu.h"

#define LOCTEXT_NAMESPACE "Menu"

void UUW_HostGame::SetInfo(const FText& Text) const
{
	TextInfo->SetText(Text);
}

void UUW_HostGame::NativeConstruct()
{
	Super::NativeConstruct();

	EditCustomName->OnTextChanged.AddUniqueDynamic(this, &UUW_HostGame::HandleEditCustomNameTextChanged);
	EditNumPlayers->OnTextChanged.AddUniqueDynamic(this, &UUW_HostGame::HandleEditNumPlayersTextChanged);
	EditNumPlayers->OnTextCommitted.AddUniqueDynamic(this, &UUW_HostGame::HandleEditNumPlayersTextCommitted);
	CheckIsPrivate->OnCheckStateChanged.AddUniqueDynamic(this, &UUW_HostGame::HandleCheckIsPrivateChanged);
	CheckEveryManForHimself->OnCheckStateChanged.AddUniqueDynamic(this, &UUW_HostGame::HandleCheckEveryManForHimselfChanged);
	CheckTeams->OnCheckStateChanged.AddUniqueDynamic(this, &UUW_HostGame::HandleCheckTeamsChanged);
	CheckCoop->OnCheckStateChanged.AddUniqueDynamic(this, &UUW_HostGame::HandleCheckCoopChanged);
	BtnStart->OnClicked().AddLambda([this] ()
	{
		GetGameInstance<UMyGameInstance>()->HostGame(GetOwningPlayer()); 
	});
	BtnBack->OnClicked().AddLambda([this] () { GetOwningPlayer()->GetHUD<AMyHUDMenu>()->MenuMultiplayerShow(); });
}

void UUW_HostGame::HandleEditCustomNameTextChanged(const FText& InText)
{
	GetGameInstance<UMyGameInstance>()->SessionConfig.CustomName = InText.ToString();
}

void UUW_HostGame::HandleEditNumPlayersTextChanged(const FText& InText)
{
	int NewMaxNumPlayers;
	if(FDefaultValueHelper::ParseInt(InText.ToString(), NewMaxNumPlayers))
	{
		GetGameInstance<UMyGameInstance>()->SessionConfig.NumMaxPlayers = NewMaxNumPlayers;
	}
}

void UUW_HostGame::HandleEditNumPlayersTextCommitted(const FText& InText, ETextCommit::Type Type)
{
	EditNumPlayers->SetText(FText::Format
		(LOCTEXT("MaxNumPlayers", "{0}")
		, GetGameInstance<UMyGameInstance>()->SessionConfig.NumMaxPlayers)
		);
}

void UUW_HostGame::HandleCheckIsPrivateChanged(bool bIsChecked)
{
	GetGameInstance<UMyGameInstance>()->SessionConfig.bPrivate = bIsChecked;
}

void UUW_HostGame::HandleCheckEveryManForHimselfChanged(bool bIsChecked)
{
	if(bIsChecked)
	{
		CheckTeams->SetIsChecked(false);
		CheckCoop->SetIsChecked(false);
		GetGameInstance<UMyGameInstance>()->SessionConfig.GameMode = EGameMode::EveryManForHimself;
	}
}

void UUW_HostGame::HandleCheckTeamsChanged(bool bIsChecked)
{
	if(bIsChecked)
	{
		CheckEveryManForHimself->SetIsChecked(false);
		CheckCoop->SetIsChecked(false);
		GetGameInstance<UMyGameInstance>()->SessionConfig.GameMode = EGameMode::Teams;
	}
}

void UUW_HostGame::HandleCheckCoopChanged(bool bIsChecked)
{
	if(bIsChecked)
	{
		CheckEveryManForHimself->SetIsChecked(false);
		CheckTeams->SetIsChecked(false);
		GetGameInstance<UMyGameInstance>()->SessionConfig.GameMode = EGameMode::Coop;
	}
}

#undef LOCTEXT_NAMESPACE
