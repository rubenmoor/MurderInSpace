// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCommonButton.h"

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
