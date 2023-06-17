#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_Orbit.generated.h"

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
class UUW_Orbit : public UUserWidget
{
	GENERATED_BODY()

public:
	// F1 marker
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FF1Marker F1Marker;

protected:
	// event handlers
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

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
private:
	void DrawOrbit(const FGeometry& AllotedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const;
};
