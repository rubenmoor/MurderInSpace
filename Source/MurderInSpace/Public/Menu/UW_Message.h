// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonTextBlock.h"
#include "HUD/MyCommonButton.h"
#include "UW_Message.generated.h"

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UUW_Message : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetMessage(const FText& Text) { TextMessage->SetText(Text); }
	void SetGobackFunc(std::function<void()> Func)
	{
		UCommonButtonBase::FCommonButtonEvent OnClicked = BtnBack->OnClicked();
		OnClicked.Clear();
		OnClicked.AddLambda(Func);
	}

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> TextMessage;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMyCommonButton> BtnBack;
};
