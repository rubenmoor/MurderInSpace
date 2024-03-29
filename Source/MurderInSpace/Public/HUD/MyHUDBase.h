// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Engine/LocalPlayer.h"

#include "MyHUDBase.generated.h"

class UUW_Settings;
class UUW_WidgetHUDBorder;
class UUW_MenuBackground;

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

	// convert from screen coordinates ([0, Width], [0, Height])
	// to center coordinates ([-0.5, 0.5], [-0.5, 0.5])
	UFUNCTION(BlueprintCallable)
	static FVector2D ScreenToCenter(const UObject* Outer, FVector2D ScreenCoords);
	
	// convert from center coordinates ([-0.5, 0.5], [-0.5, 0.5])
	// to screen coordinates ([0, Width/Scale], [0, Height/Scale])
	UFUNCTION(BlueprintCallable)
	static FVector2D CenterToScreenScaled(const UObject* Outer, FVector2D CenterCoords);
	
	// convert from center coordinates ([-0.5, 0.5], [-0.5, 0.5])
	// to screen coordinates ([0, Width], [0, Height])
	// disregarding viewport scale
	UFUNCTION(BlueprintCallable)
	static FVector2D CenterToScreen(const UObject* Outer, FVector2D CenterCoords);

	UFUNCTION()
	void SettingsShow();

	TSharedPtr<FNumberFormattingOptions> GetFODPIScale() { return FODPIScale; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D MenuBackgroundSize = { 1050, 800 };

	UFUNCTION(BlueprintCallable)
    void ScaleBorderWidget(float DPIScale);
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMG Widget Classes")
	TSubclassOf<UUW_Settings> WidgetSettingsClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UMG Widgets")
	TObjectPtr<UUW_Settings> WidgetSettings;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMG Widget Classes")
	TSubclassOf<UUW_MenuBackground> WidgetBackgroundClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UMG Widgets")
	TObjectPtr<UUW_MenuBackground> WidgetBackground;
	
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

	TSharedPtr<FNumberFormattingOptions> FOVelocity;
	TSharedPtr<FNumberFormattingOptions> FOFPS;
	TSharedPtr<FNumberFormattingOptions> FODistance;
	TSharedPtr<FNumberFormattingOptions> FODPIScale;
};
