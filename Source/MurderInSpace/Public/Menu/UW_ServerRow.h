// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "CommonTextBlock.h"
#include "UW_ServerRow.generated.h"

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UUW_ServerRow : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetPing(int32 Ping);

	UFUNCTION(BlueprintCallable)
	void SetServerName(const FText& InText);

	UFUNCTION(BlueprintCallable)
	void SetPlayerNumbers(int32 NumPlayers, int32 MaxNumPlayers);
	
protected:
	// event handlers

	virtual void NativeOnCurrentTextStyleChanged() override;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> TextPing;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> TextServerName;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> TextPlayerSymbol;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> TextPlayerCount;
};
