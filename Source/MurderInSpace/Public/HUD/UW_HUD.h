// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
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

	friend class AMyHUD;
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TextVelocitySI;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TextVelocityVCircle;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TextVelocityDirection;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCanvasPanel> CanvasCenterOfMass;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UOverlay> OverlayCenterOfMass;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ImgPointer;

	// event handlers
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
};
