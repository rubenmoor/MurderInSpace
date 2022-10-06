// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/WidgetHUDBorder.h"
#include "Blueprint/WidgetLayoutLibrary.h"

int32 UWidgetHUDBorder::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
	const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const auto PG = AllottedGeometry.ToPaintGeometry();
	const auto ViewportScale = UWidgetLayoutLibrary::GetViewportScale(GetWorld());
	const auto Vec2DSize = UWidgetLayoutLibrary::GetViewportSize(GetWorld());
	const auto Width = Vec2DSize.X / ViewportScale;
	const auto Height = Vec2DSize.Y / ViewportScale;
	const auto FGColor = InWidgetStyle.GetForegroundColor();
	const auto Thickness = 1.;
	const auto LayerIdNew = LayerId + 1;
	
	// draw HUD border
	const auto P0Left = FVector2D(Width * X0, Height * Y0);
	const auto P1Left = FVector2D(Width * X1, Height * Y1);
	const auto P2Left = FVector2D(Width * X1, Height * (1. - Y1));
	const auto P3Left = FVector2D(Width * X0, Height * (1. - Y0));
	FSlateDrawElement::MakeCubicBezierSpline(OutDrawElements, LayerIdNew, PG,
		P0Left, P1Left, P2Left, P3Left, Thickness, ESlateDrawEffect::None, FGColor);

	const auto P0Right = FVector2D(Width * (1. - X0), Height * Y0);
	const auto P1Right = FVector2D(Width * (1. - X1), Height * Y1);
	const auto P2Right = FVector2D(Width * (1. - X1), Height * (1. - Y1));
	const auto P3Right = FVector2D(Width * (1. - X0), Height * (1. - Y0));
	FSlateDrawElement::MakeCubicBezierSpline(OutDrawElements, LayerIdNew, PG,
		P0Right, P1Right, P2Right, P3Right, Thickness, ESlateDrawEffect::None, FGColor);
		
	return Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerIdNew, InWidgetStyle,
	                          bParentEnabled);
}
