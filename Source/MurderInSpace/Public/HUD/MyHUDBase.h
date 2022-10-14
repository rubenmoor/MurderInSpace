// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"

#include "MyHUDBase.generated.h"

class UWidgetHUDBorder;

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API AMyHUDBase : public AHUD
{
	GENERATED_BODY()

public:
	AMyHUDBase();

	const FLocalPlayerContext& GetLocalPlayerContext() { return LocalPlayerContext; }
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMG Widget Classes")
	TSubclassOf<UWidgetHUDBorder> WidgetHUDBorderClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UMG Widgets")
	TObjectPtr<UWidgetHUDBorder> WidgetHUDBorder;
	
    // Default values: overridden by blueprint
	// relative to viewport width: the horizontal distance from the circular HUD
	// to the viewport edge at top and bottom
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float X0 = 0.115;
	
	// relative to viewport height: the distance from the HUD to the viewport edge
	// at top and bottom
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Y0 = 0.0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float X1 = -0.02;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Y1 = 0.33;
	
	// event handlers
	
	virtual void BeginPlay() override;

	// private methods

	UFUNCTION(BlueprintCallable)
	void HideViewportParentWidgets();

	FLocalPlayerContext LocalPlayerContext;

	FNumberFormattingOptions FormattingOptions;
};
