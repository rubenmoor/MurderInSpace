// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyHUDBase.h"
#include "Components/Button.h"

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API AMyHUDMenu : public AMyHUDBase
{
	GENERATED_BODY()
	
protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UButton> BtnStart;
	
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UButton> BtnFindServer;
	
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UButton> BtnQuit;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMG Widget Classes")
	TSubclassOf<UUserWidget> UMGWidgetServerList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMG Widget Classes")
	TSubclassOf<UUserWidget> UMGWidgetMenuInGame;
	
	// event handlers

	virtual void BeginPlay() override;

	// UI event handlers
	void HandleBtnFindServerClicked();

	virtual void SetWidgetToDefault() override;
	
	UFUNCTION(BlueprintCallable)
	void SetWidgetServerList();

	UFUNCTION(BlueprintCallable)
	void SetWidgetMenuInGame();

private:
	UFUNCTION(BlueprintCallable)
	void Leave();
};
