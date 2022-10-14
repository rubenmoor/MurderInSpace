// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ScrollBox.h"
#include "CommonTextBlock.h"
#include "OnlineSessionSettings.h"
#include "HUD/MyCommonButton.h"
#include "UW_ServerList.generated.h"

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UUW_ServerList : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetStatusMessage(const FText& InText)
	{
		TextStatusMessage->SetText(InText);
		TextStatusMessage->SetVisibility(ESlateVisibility::Visible);
	}
	
	int32 SelectedIndex = -1;
	
	UFUNCTION(BlueprintCallable)
	void HideStatusMessage() { TextStatusMessage->SetVisibility(ESlateVisibility::Collapsed); }
	
	UFUNCTION(BlueprintCallable)
	void SetBtnRefreshEnabled(bool InBEnabled) const { BtnRefresh->SetIsEnabled(InBEnabled); }

	UFUNCTION(BlueprintCallable)
	void SetBtnJoinEnabled(bool InBEnabled) const { BtnJoin->SetIsEnabled(InBEnabled); }

	UFUNCTION(BlueprintCallable)
	void AddServerRow(UWidget* Row) { ScrollServers->AddChild(Row); }

	UFUNCTION(BlueprintCallable)
	void ClearServerRows() { ScrollServers->ClearChildren(); }

	UFUNCTION(BlueprintCallable)
	void DeselectAllBut(int32 Index);
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> TextStatusMessage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UScrollBox> ScrollServers;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMyCommonButton> BtnRefresh;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMyCommonButton> BtnJoin;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMyCommonButton> BtnBack;

	// event handlers

	virtual void NativeConstruct() override;
};
