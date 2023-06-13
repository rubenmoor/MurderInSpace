// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonNumericTextBlock.h"
#include "WidgetHUDBorder.h"
#include "Blueprint/UserWidget.h"
#include "Components/HorizontalBox.h"
#include "HUD/UW_Orbit.h"
#include "UW_HUD.generated.h"

class UTextBlock;
class UCanvasPanel;
class UOverlay;
class UImage;

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UUW_HUD : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCommonNumericTextBlock> TextFPS;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UHorizontalBox> HoriDistanceF1OffScreen;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCommonNumericTextBlock> TextDistanceF1OffScreen;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UHorizontalBox> HoriDistanceF1OnScreen;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCommonNumericTextBlock> TextDistanceF1OnScreen;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TextVelocitySI;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TextVelocityVCircle;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TextVelocityDirection;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TextCameraHeight;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCanvasPanel> CanvasCenterOfMass;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ImgPointer;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UUW_Orbit> WidgetOrbit;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UWidgetHUDBorder> WidgetHUDBorder;
	
	UFUNCTION(BlueprintCallable)
	void F1MarkerHide()
	{
		WidgetOrbit->F1Marker.bShow = false;
		HoriDistanceF1OnScreen->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	UFUNCTION(BlueprintCallable)
	void F1MarkerShow()
	{
		WidgetOrbit->F1Marker.bShow = true;
        HoriDistanceF1OnScreen->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ImgOrbit;
};
