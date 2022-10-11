// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "CommonTextBlock.h"
#include "MyCommonButton.generated.h"

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UMyCommonButton : public UCommonButtonBase
{
	GENERATED_BODY()

	UMyCommonButton();
protected:
	UPROPERTY(EditAnywhere)
	FText Label;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> TextBlockLabel = nullptr;

	UPROPERTY(Transient, meta=(BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> CaretRotation;
	
	// event handlers
	virtual void NativeOnCurrentTextStyleChanged() override;
	virtual void SynchronizeProperties() override;
	virtual void NativeOnFocusLost(const FFocusEvent& InFocusEvent) override;
};
