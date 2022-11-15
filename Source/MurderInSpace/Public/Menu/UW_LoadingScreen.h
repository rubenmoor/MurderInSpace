// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUD/MyCommonButton.h"
#include "CommonTextBlock.h"
#include "UW_LoadingScreen.generated.h"

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UUW_LoadingScreen : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetMessage(const FText& Text) { TextMessage->SetText(Text); }
	void SetGobackFunc(std::function<void()> GoBack)
	{
		BtnAbort->OnClicked().Clear();
		BtnAbort->OnClicked().AddLambda(GoBack);
	}
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> TextMessage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMyCommonButton> BtnAbort;
	
	// event handlers
};
