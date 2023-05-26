// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/UW_HUD.h"

#include "Actors/MyCharacter.h"
#include "Orbit/Orbit.h"
#include "Modes/MyState.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "HUD/MyHUD.h"

int32 UUW_HUD::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
                           const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
                           const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    const FPaintGeometry PG = AllottedGeometry.ToPaintGeometry();

    if(F1Marker.bShow)
    {
        MakeCircle
            (OutDrawElements
            , LayerId
            , PG
            , F1Marker.Coords
            , GetPlayerContext().GetHUD<AMyHUD>()->DistanceF1Radius
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
            , ScreenPos
            , true
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
            , true
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

    auto* Orbit = GetPlayerContext().GetPawn<AMyCharacter>()->GetOrbit();
    auto* SplineComponent = Orbit->GetSplineComponent();
    auto Points = SplineComponent->GetSplinePointsPosition().Points;
    Points.Push(FInterpCurvePoint(Points[0]));

    struct FCurvePoint2D
    {
        FCurvePoint2D(FVector2D InLoc, FVector2D InArriveTangent, FVector2D InLeaveTangent)
            : Loc(InLoc), ArriveTangent(InArriveTangent), LeaveTangent(InLeaveTangent) {};
        
        FVector2D Loc;    
        FVector2D ArriveTangent;    
        FVector2D LeaveTangent;    
    };
    TArray<FCurvePoint2D> CurvePoints;

    // if and only if all points are on-screen, we need to close the circle
    bool bAllPointsOnScreen = true;
    
    bool bThisPointOnScreen = false;

    // if the last point was on-screen, this point needs connection
    bool bLastPointOnScreen = false;
    
    const FVector2D Vec2DSize = UWidgetLayoutLibrary::GetViewportSize(GetWorld()) / UWidgetLayoutLibrary::GetViewportScale(GetWorld());
        
    TArray<FVector2D> ScreenPoints;
    for(int32 i = 0; i < Points.Num(); ++i)
    {
        FVector2D ScreenPos;
        bool bProjected = UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition
            ( GetOwningPlayer()
            , Points[i].OutVal
            , ScreenPos
            , true
            );

        bThisPointOnScreen = 
               bProjected
            && ScreenPos.X >= 0.
            && ScreenPos.X <= Vec2DSize.X
            && ScreenPos.Y >= 0.
            && ScreenPos.Y <= Vec2DSize.Y;

        // maybe causes stackoverflow
        // if(!bThisPointOnScreen)
        //     MakeCircle
        //         ( OutDrawElements
        //         , LayerId
        //         , PG
        //         , ScreenPos
        //         , 5
        //         , 1
        //         , ESlateDrawEffect::None
        //         , FLinearColor::Blue
        //         );

        auto AddCurvePoint = [&] (const FInterpCurvePoint<FVector>& Point)
        {
            FVector2D InTangentPos, OutTangentPos;

            UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition
                ( GetOwningPlayer()
                , Point.OutVal + Point.ArriveTangent
                , InTangentPos
                , true
                );
            UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition
                ( GetOwningPlayer()
                , Point.OutVal + Point.LeaveTangent
                , OutTangentPos
                , true
                );
            CurvePoints.Emplace(ScreenPos, InTangentPos - ScreenPos, OutTangentPos - ScreenPos);
        };

        if(bThisPointOnScreen || bLastPointOnScreen)
        {
            AddCurvePoint(Points[i]);
        }
        else
        {
            bAllPointsOnScreen = false;
            if(bThisPointOnScreen && !bLastPointOnScreen)
            {
                AddCurvePoint(Points[i - 1]);
            }
        }

        bLastPointOnScreen = bThisPointOnScreen;
    }

    if(CurvePoints.IsEmpty())
    {
        UE_LOG(LogMyGame, Error, TEXT("%s: curve points empty"), *GetFullName())
    }
    else
    {
        int32 iMax = CurvePoints.Num() - (bAllPointsOnScreen ? 1 : 2);
        //for(int32 i = 0; i < iMax; ++i)
        for(int32 i = 0; i < CurvePoints.Num() - 2; ++i)
        {
            MakeCircle
                ( OutDrawElements
                , LayerId
                , PG
                , CurvePoints[i].Loc
                , 5
                , 1
                , ESlateDrawEffect::None
                , FLinearColor::Green
                );
            UE_LOG(LogMyGame, Display, TEXT("Loc1: (%.1f, %.1f), LeaveTangent: (%.1f, %.1f), Loc2: (%.1f, %.1f), ArriveTangent: (%.1f, %.1f)")
                , CurvePoints[i].Loc.X, CurvePoints[i].Loc.Y
                , CurvePoints[i].LeaveTangent.X, CurvePoints[i].LeaveTangent.Y
                , CurvePoints[i + 1].Loc.X
                , CurvePoints[i + 1].Loc.Y
                , CurvePoints[i + 1].ArriveTangent.X
                , CurvePoints[i + 1].ArriveTangent.Y
                )
            FSlateDrawElement::MakeSpline
                ( OutDrawElements
                , LayerId
                , PG
                , CurvePoints[i].Loc
                , CurvePoints[i].LeaveTangent
                , CurvePoints[i + 1].Loc
                , CurvePoints[i + 1].ArriveTangent
                , 1.5
                , ESlateDrawEffect::None
                , FLinearColor::Green.CopyWithNewOpacity(0.7)
                );
        }
    }
    
    const int32 LayerIdNew = LayerId + 1;
    return Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerIdNew, InWidgetStyle,
                              bParentEnabled);
}

void UUW_HUD::MakeCircle
    ( FSlateWindowElementList& ElementList
    , uint32 LayerId
    , const FPaintGeometry& PG
    , const FVector2D& Center
    , double Radius
    , float Thickness
    , ESlateDrawEffect DrawEffect
    , const FLinearColor& Tint
    )
{
    constexpr double C = UMyState::SplineToCircle;
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
    constexpr double C = UMyState::SplineToCircle;
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
