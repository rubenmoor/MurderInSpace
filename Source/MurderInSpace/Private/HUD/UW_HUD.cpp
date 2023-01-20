// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/UW_HUD.h"

#include "Actors/Orbit.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void UUW_HUD::SetF1Marker(FVector2D InCoords)
{
    F1Marker.Coords = InCoords;
    F1Marker.bShow = true;
}

int32 UUW_HUD::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
                           const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
                           const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    const FPaintGeometry PG = AllottedGeometry.ToPaintGeometry();
    //const double ViewportScale = UWidgetLayoutLibrary::GetViewportScale(GetWorld());
    //const FVector2D Vec2DSize = UWidgetLayoutLibrary::GetViewportSize(GetWorld());

    if(F1Marker.bShow)
    {
        MakeCircle
            (OutDrawElements
            , LayerId
            , PG
            , F1Marker.Coords
            , 32
            , 1
            , ESlateDrawEffect::None
            , FLinearColor::Green
            );
    }

    const FInstanceUI InstanceUI = GEngine->GetEngineSubsystem<UMyState>()->GetInstanceUIAny(this);
    AOrbit* OrbitHovered = InstanceUI.Hovered.Orbit;
    if(IsValid(OrbitHovered))
    {
        FVector ScreenPos;
        UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPositionWithDistance
            (GetOwningPlayer()
            , OrbitHovered->GetVecR()
            , ScreenPos, false
            );
        MakeCircularFrame
            ( OutDrawElements
            , LayerId
            , PG
            , FVector2D(ScreenPos)
            , 1000. / ScreenPos.Z * InstanceUI.Hovered.Size
            , 30.
            , 3.
            , ESlateDrawEffect::None
            , FLinearColor::Green.CopyWithNewOpacity(0.7)
            );
    }

    AOrbit* OrbitSelected = InstanceUI.Selected.Orbit;
    if(IsValid(OrbitSelected))
    {
        FVector ScreenPos;
        UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPositionWithDistance
            (GetOwningPlayer()
            , OrbitSelected->GetVecR()
            , ScreenPos
            , false
            );
        MakeCircle
            ( OutDrawElements
            , LayerId
            , PG
            , FVector2D(ScreenPos)
            , 0.9 * 1000. / ScreenPos.Z * InstanceUI.Selected.Size
            , 1.
            , ESlateDrawEffect::None
            , FLinearColor::Green.CopyWithNewOpacity(0.7)
            );
    }
    
    const int32 LayerIdNew = LayerId + 1;
    return Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerIdNew, InWidgetStyle,
                              bParentEnabled);
}

void UUW_HUD::MakeCircle
    (FSlateWindowElementList& ElementList
    , uint32 LayerId
    , const FPaintGeometry& PG
    , const FVector2D& Center
    , double Radius
    , float Thickness
    , ESlateDrawEffect DrawEffect
    , const FLinearColor& Tint
    )
{
    constexpr double C = AOrbit::SplineToCircle;
    const FVector2D P1 = Center + FVector2D(-Radius, 0);
    const FVector2D T1 = FVector2D(0, -Radius) * C;
    const FVector2D P2 = Center + FVector2D(0, -Radius);
    const FVector2D T2 = FVector2D(Radius, 0) * C;
    const FVector2D P3 = Center + FVector2D(Radius, 0);
    const FVector2D T3 = FVector2D(0, Radius) * C;
    const FVector2D P4 = Center + FVector2D(0, Radius);
    const FVector2D T4 = FVector2D(-Radius, 0) * C;
    
    FSlateDrawElement::MakeSpline(ElementList, LayerId, PG, P1, T1, P2, T2, Thickness, DrawEffect, Tint);
    FSlateDrawElement::MakeSpline(ElementList, LayerId, PG, P2, T2, P3, T3, Thickness, DrawEffect, Tint);
    FSlateDrawElement::MakeSpline(ElementList, LayerId, PG, P3, T3, P4, T4, Thickness, DrawEffect, Tint);
    FSlateDrawElement::MakeSpline(ElementList, LayerId, PG, P4, T4, P1, T1, Thickness, DrawEffect, Tint);
}

void UUW_HUD::MakeCircularFrame(FSlateWindowElementList& ElementList, uint32 LayerId, const FPaintGeometry& PG,
    const FVector2D& Center, double Radius, double AngularWidth, float Thickness, ESlateDrawEffect DrawEffect, const FLinearColor& Tint)
{
    constexpr double C = AOrbit::SplineToCircle;
    for(double Alpha = 45; Alpha < 360; Alpha += 90)
    {
        const double AlphaStart = Alpha - AngularWidth / 2.;
        const double AlphaEnd   = Alpha + AngularWidth / 2.;
        const FVector2D Start  = Center + FVector2D(Radius, 0).GetRotated(AlphaStart);
        const FVector2D End    = Center + FVector2D(Radius, 0).GetRotated(AlphaEnd);
        const FVector2D StartT = FVector2D(0, Radius).GetRotated(AlphaStart) * C * AngularWidth / 90.;
        const FVector2D EndT   = FVector2D(0, Radius).GetRotated(AlphaEnd  ) * C * AngularWidth / 90.;
        FSlateDrawElement::MakeSpline(ElementList, LayerId, PG, Start, StartT, End, EndT, Thickness, DrawEffect, Tint);
    }
}
