// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonNumericTextBlock.h"
#include "Blueprint/UserWidget.h"
#include "Components/HorizontalBox.h"
#include "UW_HUD.generated.h"

class UTextBlock;
class UCanvasPanel;
class UOverlay;
class UImage;

USTRUCT(BlueprintType)
struct FF1Marker
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bShow = false;

	// screen coordinates
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D Coords = FVector2D::Zero();
};

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UUW_HUD : public UUserWidget
{
	GENERATED_BODY()

	friend class AMyHUD;

public:
	UFUNCTION(BlueprintCallable)
	void F1MarkerHide()
	{
		F1Marker.bShow = false;
		HoriDistanceF1OnScreen->SetVisibility(ESlateVisibility::Collapsed);
	}
	UFUNCTION(BlueprintCallable)
	void F1MarkerShow()
	{
		F1Marker.bShow = true;
        HoriDistanceF1OnScreen->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

protected:
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

	// event handlers
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	// F1 marker
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FF1Marker F1Marker;

	static void MakeCircle
		( FSlateWindowElementList& ElementList
		, uint32 LayerId
		, const FPaintGeometry& PG
		, const FVector2D& Center
		, double Radius
		, float Thickness = 0
		, ESlateDrawEffect DrawEffect = ESlateDrawEffect::None
		, const FLinearColor& Tint = FLinearColor::White
		);
		
	static void MakeCircularFrame
		( FSlateWindowElementList& ElementList
		, uint32 LayerId
		, const FPaintGeometry& PG
		, const FVector2D& Center
		, double Radius
		, double AngularWidth
		, float Thickness = 0
		, ESlateDrawEffect DrawEffect = ESlateDrawEffect::None
		, const FLinearColor& Tint = FLinearColor::White
		);
};
