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
#include "Kismet/GameplayStatics.h"

int32 UUW_Orbit::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
                             const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
                             const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    const FPaintGeometry PG = AllottedGeometry.ToPaintGeometry();
	const auto FGColor = InWidgetStyle.GetForegroundColor();

    auto* PC = GetOwningPlayer();
    const float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(GetWorld());

    if(F1Marker.bShow)
    {
        MakeCircle
            (OutDrawElements
            , LayerId
            , PG
            , F1Marker.Coords / ViewportScale
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
        PC->ProjectWorldLocationToScreenWithDistance
            ( OrbitHovered->GetVecR()
            , ScreenPos
            , true
            );
        MakeCircularFrame
            ( OutDrawElements
            , LayerId
            , PG
            , FVector2D(ScreenPos) / ViewportScale
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
        PC->ProjectWorldLocationToScreenWithDistance
            ( OrbitSelected->GetVecR()
            , ScreenPos
            , true
            );
        MakeCircle
            ( OutDrawElements
            , LayerId
            , PG
            , FVector2D(ScreenPos) / ViewportScale
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
    
    const FVector2D Vec2DSize = UWidgetLayoutLibrary::GetViewportSize(GetWorld());
    
    // "on-screen" really means on screen within the given tolerance
    //constexpr float Tolerance = 0.1;
    constexpr float Tolerance = 0.;
    const float ToleranceX = Vec2DSize.X * Tolerance;
    const float ToleranceY = Vec2DSize.Y * Tolerance;
    const float XMin = -ToleranceX;
    const float XMax = Vec2DSize.X + ToleranceX;
    const float YMin = -ToleranceY;
    const float YMax = Vec2DSize.Y + ToleranceY;
        
    for(int32 i = 0; i < Points.Num(); ++i)
    {
        FVector2D ScreenPos;
        bool bProjected = PC->ProjectWorldLocationToScreen
            ( Points[i].OutVal
            , ScreenPos
            , true
            );
        // bool bProjected = UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition
        //     ( PC
        //     , Points[i].OutVal
        //     , ScreenPos
        //     , true
        //     );
        // ScreenPos *= UWidgetLayoutLibrary::GetViewportScale(GetWorld());

        bool bOnScreen = 
               bProjected
            && ScreenPos.X >= XMin
            && ScreenPos.X <= XMax
            && ScreenPos.Y >= YMin
            && ScreenPos.Y <= YMax;

        CurvePoints[i] = FCurvePoint2D(ScreenPos, bOnScreen, Points[i].OutVal, Points[i].ArriveTangent, Points[i].LeaveTangent);
    }

    // calculate the world position of the visible screen corners on the xy-plane
    FVector TopLeft, TopLeftDirection, BottomLeft, BottomLeftDirection, TopRight, TopRightDirection, BottomRight, BottomRightDirection;
    UGameplayStatics::DeprojectScreenToWorld(PC, {XMin, YMin}, TopLeft, TopLeftDirection);
    UGameplayStatics::DeprojectScreenToWorld(PC, {XMin, YMax}, BottomLeft, BottomLeftDirection);
    UGameplayStatics::DeprojectScreenToWorld(PC, {XMax, YMin}, TopRight, TopRightDirection);
    UGameplayStatics::DeprojectScreenToWorld(PC, {XMax, YMax}, BottomRight, BottomRightDirection);
    
    const float AlphaTL = -TopLeft.Z / TopLeftDirection.Z;
    const FVector2D TopLeftZ0(TopLeft.X + AlphaTL * TopLeftDirection.X, TopLeft.Y + AlphaTL * TopLeftDirection.Y);
    const float AlphaBL = -BottomLeft.Z / BottomLeftDirection.Z;
    const FVector2D BottomLeftZ0(BottomLeft.X + AlphaBL * BottomLeftDirection.X, BottomLeft.Y + AlphaBL * BottomLeftDirection.Y);
    const float AlphaTR = -TopRight.Z / TopRightDirection.Z;
    const FVector2D TopRightZ0(TopRight.X + AlphaTR * TopRightDirection.X, TopRight.Y + AlphaTR * TopRightDirection.Y);
    const float AlphaBR = -BottomRight.Z / BottomRightDirection.Z;
    const FVector2D BottomRightZ0(BottomRight.X + AlphaBR * BottomRightDirection.X, BottomRight.Y + AlphaBR * BottomRightDirection.Y);

    const float MTop = (TopLeft.X - TopRight.X) / (TopLeft.Y - TopRight.Y);
    const float MBottom = (BottomLeft.X - BottomRight.X) / (BottomLeft.Y - BottomRight.Y);
    const float MLeft = (TopLeftZ0.Y - BottomLeftZ0.Y) / (TopLeftZ0.X - BottomLeftZ0.X);
    const float MRight = (TopRightZ0.Y - BottomRightZ0.Y) / (TopRightZ0.X - BottomRightZ0.X);
    UE_LOG(LogMyGame, Warning, TEXT("%.3f, %.3f, %.3f, %.3f"), MTop, MBottom, MLeft, MRight)

    FVector2D DebugTL, DebugTR, DebugBL, DebugBR;
    PC->ProjectWorldLocationToScreen
        ( FVector(TopLeftZ0.X, TopLeftZ0.Y, 0.)
        , DebugTL
        , true
        );
    MakeCircle(OutDrawElements, LayerId, PG, DebugTL / ViewportScale, 100, 2);
    PC->ProjectWorldLocationToScreen
        ( FVector(TopRightZ0.X, TopRightZ0.Y, 0.)
        , DebugTR
        , true
        );
    MakeCircle(OutDrawElements, LayerId, PG, DebugTR / ViewportScale, 100, 2);
    PC->ProjectWorldLocationToScreen
        ( FVector(BottomLeftZ0.X, BottomLeftZ0.Y, 0.)
        , DebugBL
        , true
        );
    MakeCircle(OutDrawElements, LayerId, PG, DebugBL / ViewportScale, 100, 2);
    PC->ProjectWorldLocationToScreen
        ( FVector(BottomRightZ0.X, BottomRightZ0.Y, 0.)
        , DebugBR
        , true
        );
    MakeCircle(OutDrawElements, LayerId, PG, DebugBR / ViewportScale, 100, 2);
    
    for(int32 i = 0; i < CurvePoints.Num(); ++i)
    {
        FCurvePoint2D ThisCurvePoint = CurvePoints[i];
        FCurvePoint2D NextCurvePoint = CurvePoints[(i + 1) % CurvePoints.Num()];

        // simple case: this point and nex point on-screen
        if  (
               ThisCurvePoint.bOnScreen
            && NextCurvePoint.bOnScreen
            )
        {
            FVector2D LeaveTangentPos, ArriveTangentPos;
            PC->ProjectWorldLocationToScreen
                ( ThisCurvePoint.Pos + ThisCurvePoint.LeaveTangent
                , LeaveTangentPos
                , true
                );
            PC->ProjectWorldLocationToScreen
                ( NextCurvePoint.Pos + NextCurvePoint.ArriveTangent
                , ArriveTangentPos
                , true
                );
            FSlateDrawElement::MakeSpline
                ( OutDrawElements
                , LayerId
                , PG
                , ThisCurvePoint.Loc / ViewportScale
                , (LeaveTangentPos - ThisCurvePoint.Loc) / ViewportScale
                , NextCurvePoint.Loc / ViewportScale
                , (ArriveTangentPos - NextCurvePoint.Loc) / ViewportScale
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
