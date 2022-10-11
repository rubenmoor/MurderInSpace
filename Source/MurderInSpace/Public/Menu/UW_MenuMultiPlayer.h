// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_MenuMultiPlayer.generated.h"

class UMyCommonButton;

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UUW_MenuMultiPlayer : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMyCommonButton> BtnJoin;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMyCommonButton> BtnCreateNew;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMyCommonButton> BtnLoad;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMyCommonButton> BtnBack;

	// event handlers

	virtual void NativeConstruct() override;
};
