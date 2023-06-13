// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Chaos/ObjectPool.h"
#include "UW_MenuInGame.generated.h"

class UMyCommonButton;
/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UUW_MenuInGame : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMyCommonButton> BtnResume;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMyCommonButton> BtnSettings;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMyCommonButton> BtnLeave;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMyCommonButton> BtnQuit;

	// event handlers
	virtual void NativeConstruct() override;
};
