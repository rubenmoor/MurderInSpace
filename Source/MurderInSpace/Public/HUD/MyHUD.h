// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyHUDBase.h"
#include "HUD/UW_HUD.h"
#include "HUD/UW_MenuInGame.h"
#include "MyHUD.generated.h"

class ACharacterInSpace;

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API AMyHUD : public AMyHUDBase
{
	GENERATED_BODY()

	friend class UMyGameInstance;

public:
	UFUNCTION(BlueprintCallable)
    void InGameMenuShow();
	
	UFUNCTION(BlueprintCallable)
	void InGameMenuHide();
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMG Widget Classes")
	TSubclassOf<UUW_HUD> WidgetHUDClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="UMG Widgets")
	TObjectPtr<UUW_HUD> WidgetHUD;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<ACharacterInSpace> MyCharacter;

	// in game menu

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMG Widget Classes")
	TSubclassOf<UUW_MenuInGame> WidgetMenuInGameClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="UMG Widgets")
	TObjectPtr<UUW_MenuInGame> WidgetMenuInGame;
	
	// event handlers

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	
};
	
