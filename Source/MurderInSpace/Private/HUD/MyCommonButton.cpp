// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/MyCommonButton.h"

UMyCommonButton::UMyCommonButton()
{
	OnFocusReceived().AddLambda([this] ()
	{
		PlayAnimation(CaretRotation, 0, 0);
	});
	// OnHovered().AddLambda([this] ()
	// {
	// 	SetFocus();
	// });
	// OnUnhovered().AddLambda([this] ()
	// {
	// 	
	// });
}

void UMyCommonButton::NativeOnCurrentTextStyleChanged()
{
	Super::NativeOnCurrentTextStyleChanged();
	TextBlockLabel->SetStyle(GetCurrentTextStyleClass());
}

void UMyCommonButton::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	if(!TextBlockLabel)
	{
		UE_LOG
			( LogSlate
			, Warning
			, TEXT("%s: UMyCommonButton::SynchronizeProperties: TextBlockLabel null, skipping")
			, *GetFullName()
			)
		return;
	}
	TextBlockLabel->SetText(Label);
}

void UMyCommonButton::NativeOnFocusLost(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnFocusLost(InFocusEvent);
	UE_LOG(LogSlate, Warning, TEXT("focus lost"))
	StopAnimation(CaretRotation);
}
