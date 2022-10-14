// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu/UW_ServerRow.h"

#include "CommonTextBlock.h"

#define LOCTEXT_NAMESPACE "Menu"

void UUW_ServerRow::SetPing(int32 Ping)
{
	TextPing->SetText(FText::Format(LOCTEXT("ping", "{0}"), Ping));
}

void UUW_ServerRow::SetServerName(const FText& InText)
{
	TextServerName->SetText(InText);
}

void UUW_ServerRow::SetPlayerNumbers(int32 NumPlayers, int32 MaxNumPlayers)
{
	TextPlayerCount->SetText(FText::Format(LOCTEXT("Playercount", "{0} / {1}"), NumPlayers, MaxNumPlayers));
}

void UUW_ServerRow::NativeOnCurrentTextStyleChanged()
{
	Super::NativeOnCurrentTextStyleChanged();

	TextPing->SetStyle(GetCurrentTextStyleClass());
	TextServerName->SetStyle(GetCurrentTextStyleClass());
	TextPlayerSymbol->SetStyle(GetCurrentTextStyleClass());
	FSlateFontInfo SlateFontInfo;
	SlateFontInfo.TypefaceFontName = FName(TEXT("UnicodeAll"));
	TextPlayerSymbol->SetFont(SlateFontInfo);
	TextPlayerCount->SetStyle(GetCurrentTextStyleClass());
}

#undef LOCTEXT_NAMESPACE
