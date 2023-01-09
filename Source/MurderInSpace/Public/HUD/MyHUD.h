// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyHUDBase.h"
#include "HUD/UW_HUD.h"
#include "HUD/UW_MenuInGame.h"
#include "Misc/EnumClassFlags.h"
#include "MyHUD.generated.h"

UENUM(meta=(Bitflags))
enum class EHUDReady : uint8
{
	None = 0,
	InternalReady  = 1 << 0,
	OrbitReady     = 1 << 1,
	PawnOrbitReady = 1 << 2,
	
	All = InternalReady | OrbitReady | PawnOrbitReady
};
ENUM_CLASS_FLAGS(EHUDReady)

class AMyCharacter;

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

	void SetReadyFlags(EHUDReady ReadyFlags);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMG Widget Classes")
	TSubclassOf<UUW_HUD> WidgetHUDClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="UMG Widgets")
	TObjectPtr<UUW_HUD> WidgetHUD;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AMyCharacter> MyCharacter;

	// in game menu

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMG Widget Classes")
	TSubclassOf<UUW_MenuInGame> WidgetMenuInGameClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="UMG Widgets")
	TObjectPtr<UUW_MenuInGame> WidgetMenuInGame;
	
	// event handlers

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// private members

	EHUDReady HUDReady = EHUDReady::None;
};
	
