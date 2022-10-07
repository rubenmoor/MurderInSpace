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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMG Widget Classes")
	TSubclassOf<UUserWidget> WidgetMainMenuClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UMG Widgets")
	TObjectPtr<UUserWidget> WidgetMainMenu;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMG Widget Classes")
	TSubclassOf<UUserWidget> WidgetServerListClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UMG Widgets")
	TObjectPtr<UUserWidget> WidgetServerList;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMG Widget Classes")
	TSubclassOf<UUserWidget> WidgetLoadingScreenClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UMG Widgets")
	TObjectPtr<UUserWidget> WidgetLoadingScreen;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMG Widget Classes")
	TSubclassOf<UUserWidget> WidgetMessageClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UMG Widgets")
	TObjectPtr<UUserWidget> WidgetMessage;
	
	// event handlers

	virtual void BeginPlay() override;

	// private methods
	
	UFUNCTION(BlueprintCallable)
	void ServerListShow();
	
	UFUNCTION(BlueprintCallable)
	void MainMenuShow();

	UFUNCTION(BlueprintCallable)
	void LoadingScreenShow(FText StrMessage);

	void MessageShow(FText StrMessage, TFunctionRef<void()> FuncGoBack);
	FDelegateHandle DHMessageShowGoBack;
};
