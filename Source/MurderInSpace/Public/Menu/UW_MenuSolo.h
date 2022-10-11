// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_MenuSolo.generated.h"

class UMyCommonButton;

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UUW_MenuSolo : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMyCommonButton> BtnContinue;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMyCommonButton> BtnStartNew;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMyCommonButton> BtnLoad;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMyCommonButton> BtnBack;

	// event handlers

	virtual void NativeConstruct() override;
};
