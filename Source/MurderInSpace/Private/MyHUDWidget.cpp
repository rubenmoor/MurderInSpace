// Fill out your copyright notice in the Description page of Project Settings.


#include "MyHUDWidget.h"

#include "MyHUDBase.h"
#include "Blueprint/WidgetLayoutLibrary.h"

int32 UMyHUDWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
                                const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
                                const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	// const auto PG = AllottedGeometry.ToPaintGeometry();
	// const auto ViewportScale = UWidgetLayoutLibrary::GetViewportScale(GetWorld());
	// const auto Vec2DSize = UWidgetLayoutLibrary::GetViewportSize(GetWorld());
	// const auto Width = Vec2DSize.X / ViewportScale;
	// const auto Height = Vec2DSize.Y / ViewportScale;
	// const auto X0 = .2;
	// const auto Y0 = .2;
	// const auto P0 = FVector2D(Width * X0, Height * Y0);
	// const auto P1 = FVector2D(Width * X0, Height * (1. - Y0));
	// const auto P2 = FVector2D(Width * (1. - X0), Height * (1. - Y0));
	// const auto P3 = FVector2D(Width * (1. - X0), Height * Y0);
	const auto LayerIdNew = LayerId + 1;
	// 
	// const TArray<FVector2D> Points = { P0, P1, P2, P3, P0 };
	// FSlateDrawElement::MakeLines(OutDrawElements, LayerIdNew, PG, Points);
	return Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerIdNew, InWidgetStyle,
	                          bParentEnabled);
}
