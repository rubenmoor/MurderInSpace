// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/SlateEnums.h"
#include "UW_HostGame.generated.h"

class UCommonTextBlock;
class UCheckBox;
class UEditableTextBox;
class UMyCommonButton;

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UUW_HostGame : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetInfo(const FText& Text) const;
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> TextInfo;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableTextBox> EditCustomName;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCheckBox> CheckIsPrivate;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableTextBox> EditNumPlayers;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCheckBox> CheckEveryManForHimself;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCheckBox> CheckTeams;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCheckBox> CheckCoop;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMyCommonButton> BtnStart;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMyCommonButton> BtnBack;

	// event handlers
	virtual void NativeConstruct() override;

	// host game session config
	// unfortunately, EditableTextBox::OnTxtChanged is a dynamic delegate and doesn't allow lambdas
	
	UFUNCTION()
	void HandleEditCustomNameTextChanged(const FText& InText);
	
	UFUNCTION()
	void HandleEditNumPlayersTextChanged(const FText& InText);
	
	UFUNCTION()
	void HandleEditNumPlayersTextCommitted(const FText& InText, ETextCommit::Type Type);

	UFUNCTION()
	void HandleCheckIsPrivateChanged(bool bIsChecked);

	UFUNCTION()
	void HandleCheckEveryManForHimselfChanged(bool bIsChecked);

	UFUNCTION()
	void HandleCheckTeamsChanged(bool bIsChecked);

	UFUNCTION()
	void HandleCheckCoopChanged(bool bIsChecked);
};
