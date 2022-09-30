// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyHUDBase.h"

#include "MyHUDMenu.generated.h"

class UButton;
/**
 * 
 */
UCLASS()
class MURDERINSPACE_API AMyHUDMenu : public AMyHUDBase
{
	GENERATED_BODY()

	friend class UMyGameInstance;
protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="User Widget")
	TSubclassOf<UUserWidget> WidgetMainMenuClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="User Widget")
	TObjectPtr<UUserWidget> WidgetMainMenu;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="User Widget")
	TSubclassOf<UUserWidget> WidgetServerListClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="User Widget")
	TObjectPtr<UUserWidget> WidgetServerList;
	
	// event handlers

	virtual void BeginPlay() override;

	// private methods
	
	UFUNCTION(BlueprintCallable)
	void ServerListShow();
	
	UFUNCTION(BlueprintCallable)
	void MainMenuShow();

	UFUNCTION()
	void BtnStartClicked();
};
