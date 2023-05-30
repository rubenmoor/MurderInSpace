// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/UW_Orbit.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

#include "Actors/MyCharacter.h"
#include "Orbit/Orbit.h"
#include "Modes/MyState.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "HUD/MyHUD.h"

int32 UUW_Orbit::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
                             const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
                             const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    const FPaintGeometry PG = AllottedGeometry.ToPaintGeometry();
	const auto FGColor = InWidgetStyle.GetForegroundColor();

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
            , FGColor
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
            , FGColor
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
            , FGColor
            );
    }

    TArray<FInterpCurvePoint<FVector>> Points;
#if WITH_EDITOR
    if(GetWorld()->WorldType == EWorldType::PIE && !GEditor->IsSimulateInEditorInProgress())
    {
        Points = GetPlayerContext().GetPawn<AMyCharacter>()->GetOrbit()
            ->GetSplineComponent()->GetSplinePointsPosition().Points;
    }
#else
    Points = GetPlayerContext().GetPawn<AMyCharacter>()->GetOrbit()
        ->GetSplineComponent()->GetSplinePointsPosition().Points;
#endif

    struct FCurvePoint2D
    {
        FCurvePoint2D() { bOnScreen = false; }
        FCurvePoint2D(FVector2D InLoc, bool InOnScreen, FVector InPos, FVector InArriveTangent, FVector InLeaveTangent)
            : Loc(InLoc), bOnScreen(InOnScreen), Pos(InPos), ArriveTangent(InArriveTangent), LeaveTangent(InLeaveTangent) {}
        
        FVector2D Loc;
        bool bOnScreen;
        FVector Pos;
        FVector ArriveTangent;    
        FVector LeaveTangent;
    };
    TArray<FCurvePoint2D> CurvePoints;
    CurvePoints.Reserve(Points.Num());
    CurvePoints.SetNum(Points.Num());
    
    const FVector2D Vec2DSize = UWidgetLayoutLibrary::GetViewportSize(GetWorld()) / UWidgetLayoutLibrary::GetViewportScale(GetWorld());
        
    for(int32 i = 0; i < Points.Num(); ++i)
    {
        FVector2D ScreenPos;
        bool bProjected = UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition
            ( GetOwningPlayer()
            , Points[i].OutVal
            , ScreenPos
            , true
            );

        // "on-screen" really means on screen within the given tolerance
        const float ToleranceX = Vec2DSize.X;
        const float ToleranceY = Vec2DSize.Y;
        bool bOnScreen = 
               bProjected
            && ScreenPos.X >= -ToleranceX
            && ScreenPos.X <= Vec2DSize.X + ToleranceX
            && ScreenPos.Y >= -ToleranceY
            && ScreenPos.Y <= Vec2DSize.Y + ToleranceY;

        CurvePoints[i] = FCurvePoint2D(ScreenPos, bOnScreen, Points[i].OutVal, Points[i].ArriveTangent, Points[i].LeaveTangent);
    }

    for(int32 i = 0; i < CurvePoints.Num(); ++i)
    {
        FCurvePoint2D ThisCurvePoint = CurvePoints[i];
        FCurvePoint2D NextCurvePoint = CurvePoints[(i + 1) % CurvePoints.Num()];
        
        if  (
               ThisCurvePoint.bOnScreen
            && NextCurvePoint.bOnScreen
            )
        {
            FVector2D LeaveTangentPos, ArriveTangentPos;
            UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition
                ( GetOwningPlayer()
                , ThisCurvePoint.Pos + ThisCurvePoint.LeaveTangent
                , LeaveTangentPos
                , true
                );
            UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition
                ( GetOwningPlayer()
                , NextCurvePoint.Pos + NextCurvePoint.ArriveTangent
                , ArriveTangentPos
                , true
                );
            FSlateDrawElement::MakeSpline
                ( OutDrawElements
                , LayerId
                , PG
                , ThisCurvePoint.Loc
                , LeaveTangentPos - ThisCurvePoint.Loc
                , NextCurvePoint.Loc
                , ArriveTangentPos - NextCurvePoint.Loc
                , 1
                , ESlateDrawEffect::None
                , FGColor
                );
        }
        else
        {
            if(ThisCurvePoint.bOnScreen /* next isn't */)
            {
                
            }
            else if(NextCurvePoint.bOnScreen /* this isn't */ )
            {
                
            }
            else /* neither is */
            {
                FCurvePoint2D PrevCurvePoint = CurvePoints[(i + CurvePoints.Num() - 1) % CurvePoints.Num()];
                if(PrevCurvePoint.bOnScreen)
                {
                    
                }
                else if(false)
                // TODO: 
                // neither this, previous, nor next point is on screen - but this point is part of the closest pair
                {
                    
                }
            }
        }
    }
    
    const int32 LayerIdNew = LayerId + 1;
    return Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerIdNew, InWidgetStyle,
                              bParentEnabled);
}

void UUW_Orbit::MakeCircle
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

void UUW_Orbit::MakeCircularFrame(FSlateWindowElementList& ElementList, uint32 LayerId, const FPaintGeometry& PG,
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
