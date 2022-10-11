// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ScrollBox.h"
#include "CommonTextBlock.h"
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
	void SetStatusMessage(const FText& InText) { TextStatusMessage->SetText(InText); }
	
	UFUNCTION(BlueprintCallable)
	void SetBtnRefreshEnabled(bool InBEnabled) { BtnRefresh->SetIsEnabled(InBEnabled); }
		
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> TextStatusMessage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UScrollBox> ScrollServers;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMyCommonButton> BtnRefresh;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMyCommonButton> BtnBack;

	// event handlers

	virtual void NativeConstruct() override;
};
