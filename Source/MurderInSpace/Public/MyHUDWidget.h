// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyHUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UMyHUDWidget : public UUserWidget
{
	GENERATED_BODY()

	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

public:
	
	void SetParams(float _X0, float _Y0, float _X1, float _Y1)
	{
		X0 = _X0;
		Y0 = _Y0;
		X1 = _X1;
		Y1 = _Y1;
	};
	
private:
	float X0;
	float Y0;
	float X1;
	float Y1;
};
