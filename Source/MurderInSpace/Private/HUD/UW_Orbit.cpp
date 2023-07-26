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
#include "Logging/StructuredLog.h"
#include "Modes/MyPlayerController.h"

int32 UUW_Orbit::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
                             const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
                             const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    const FPaintGeometry PG = AllottedGeometry.ToPaintGeometry();
	const auto FGColor = InWidgetStyle.GetForegroundColor();

    auto* PC = Cast<AMyPlayerController>(GetOwningPlayer());
    if(!IsValid(PC))
    {
        UE_LOGFMT(LogMyGame, Error, "{THIS}: {FUNCTION}: Player Controller invalid, shouldn't be ticking", *GetFullName(), __FUNCTION__);
        return LayerId;
    }
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

    const auto Hovered = PC->GetHovered();
    if(IsValid(Hovered.Orbit))
    {
        FVector ScreenPos;
        PC->ProjectWorldLocationToScreenWithDistance
            ( Hovered.Orbit->GetVecR()
            , ScreenPos
            , true
            );
        MakeCircularFrame
            ( OutDrawElements
            , LayerId
            , PG
            , FVector2D(ScreenPos) / ViewportScale
            , 1000. / ScreenPos.Z * Hovered.Size
            , 30.
            , 3.
            , ESlateDrawEffect::None
            , FGColor
            );
    }

    const auto Selected = PC->GetSelected();
    if(IsValid(Selected.Orbit))
    {
        FVector ScreenPos;
        PC->ProjectWorldLocationToScreenWithDistance
            ( Selected.Orbit->GetVecR()
            , ScreenPos
            , true
            );
        MakeCircle
            ( OutDrawElements
            , LayerId
            , PG
            , FVector2D(ScreenPos) / ViewportScale
            , 0.9 * 1000. / ScreenPos.Z * Selected.Size
            , 1.
            , ESlateDrawEffect::None
            , FGColor
            );
    }

    //DrawOrbit(AllottedGeometry, OutDrawElements, LayerId, InWidgetStyle);
    
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

void UUW_Orbit::DrawOrbit(const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const
{
    auto* PC = GetOwningPlayer();
    const float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(GetWorld());
    const FPaintGeometry PG = AllottedGeometry.ToPaintGeometry();
	const auto FGColor = InWidgetStyle.GetForegroundColor();
    
    TArray<FInterpCurvePoint<FVector>> Points;
    USplineComponent* Spline = nullptr;
#if WITH_EDITOR
    if(GetWorld()->WorldType == EWorldType::PIE && !GEditor->IsSimulateInEditorInProgress())
    {
        Spline = GetPlayerContext().GetPawn<AMyCharacter>()->GetOrbit() ->GetSplineComponent();
        Points = Spline->GetSplinePointsPosition().Points;
    }
#else
    Spline = GetPlayerContext().GetPawn<AMyCharacter>()->GetOrbit() ->GetSplineComponent();
    Points = Spline->GetSplinePointsPosition().Points;
#endif

    struct FCurvePoint2D
    {
        FCurvePoint2D() { bOnScreen = false; }
        FCurvePoint2D(FVector2D InLoc, bool InOnScreen, FVector InPos, FVector InArriveTangent, FVector InLeaveTangent)
            : Loc(InLoc), bOnScreen(InOnScreen) {}
        
        FVector2D Loc;
        bool bOnScreen;
    };
    TArray<FCurvePoint2D> CurvePoints;
    CurvePoints.Reserve(Points.Num());
    CurvePoints.SetNum(Points.Num());
    
    const FVector2D Vec2DSize = UWidgetLayoutLibrary::GetViewportSize(GetWorld());
    
    // "on-screen" really means on screen within the given tolerance
    //constexpr float Tolerance = 0.1;
    constexpr float Tolerance = -0.2;
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

    TArray<FVector2D> DebugFramePoints = { {XMin, YMin}, {XMin, YMax}, {XMax, YMax}, {XMax, YMin}, {XMin, YMin}};
    for(auto& Point : DebugFramePoints)
    {
        Point /= ViewportScale;
    }
    FSlateDrawElement::MakeLines(OutDrawElements, LayerId, PG, DebugFramePoints);
    
    const float AlphaTL = -TopLeft.Z / TopLeftDirection.Z;
    const FVector2D TopLeftZ0(TopLeft.X + AlphaTL * TopLeftDirection.X, TopLeft.Y + AlphaTL * TopLeftDirection.Y);
    const float AlphaBL = -BottomLeft.Z / BottomLeftDirection.Z;
    const FVector2D BottomLeftZ0(BottomLeft.X + AlphaBL * BottomLeftDirection.X, BottomLeft.Y + AlphaBL * BottomLeftDirection.Y);
    const float AlphaTR = -TopRight.Z / TopRightDirection.Z;
    const FVector2D TopRightZ0(TopRight.X + AlphaTR * TopRightDirection.X, TopRight.Y + AlphaTR * TopRightDirection.Y);
    const float AlphaBR = -BottomRight.Z / BottomRightDirection.Z;
    const FVector2D BottomRightZ0(BottomRight.X + AlphaBR * BottomRightDirection.X, BottomRight.Y + AlphaBR * BottomRightDirection.Y);

    // screen limits in the z-plane in world-space 
    const float CTop = TopLeftZ0.X; // MTop = 0
    const float CBottom = BottomLeftZ0.X; // MBottom = 0
    const float MLeft = (TopLeftZ0.Y - BottomLeftZ0.Y) / (TopLeftZ0.X - BottomLeftZ0.X);
    const float CLeft = TopLeftZ0.Y - MLeft * TopLeftZ0.X;
    const float MRight = -MLeft;
    const float CRight = TopRightZ0.Y - MRight * TopRightZ0.X;

    for(int32 i = 0; i < CurvePoints.Num(); ++i)
    {
        FCurvePoint2D ThisCurvePoint = CurvePoints[i];
        FInterpCurvePoint<FVector> ThisPoint = Points[i];
        FCurvePoint2D NextCurvePoint = CurvePoints[(i + 1) % CurvePoints.Num()];
        FInterpCurvePoint<FVector> NextPoint = Points[(i + 1) % Points.Num()];

        // final
        //bool bDrawSpline;
        // debugging
        bool bDrawSpline = false;
        
        FVector2D Start, StartDir, End, EndDir;
        
        // simple case: this point and next point on-screen
        if  (
               ThisCurvePoint.bOnScreen
            && NextCurvePoint.bOnScreen
            )
        {
            FVector2D LeaveTangentPos, ArriveTangentPos;
            PC->ProjectWorldLocationToScreen
                ( ThisPoint.OutVal + ThisPoint.LeaveTangent
                , LeaveTangentPos
                , true
                );
            PC->ProjectWorldLocationToScreen
                ( NextPoint.OutVal + NextPoint.ArriveTangent
                , ArriveTangentPos
                , true
                );
            Start = ThisCurvePoint.Loc;
            StartDir = LeaveTangentPos - Start;
            End = NextCurvePoint.Loc;
            EndDir = ArriveTangentPos - End;
            bDrawSpline = true;
        }
        else
        {
            if(ThisCurvePoint.bOnScreen /* next isn't */)
            {
                FVector2D LeaveTangentPos, ArriveTangentPos;
                PC->ProjectWorldLocationToScreen
                    ( ThisPoint.OutVal + ThisPoint.LeaveTangent
                    , LeaveTangentPos
                    , true
                    );
                Start = ThisCurvePoint.Loc;
                StartDir = LeaveTangentPos - NextCurvePoint.Loc;
                
                const float ITopY = ThisPoint.OutVal.Y + (CTop - ThisPoint.OutVal.X) / ThisPoint.LeaveTangent.X * ThisPoint.LeaveTangent.Y;
                const float IBottomY = ThisPoint.OutVal.Y + (CBottom - ThisPoint.OutVal.X) / ThisPoint.LeaveTangent.X * ThisPoint.LeaveTangent.Y;
                const bool bUpwards = ThisPoint.LeaveTangent.X > 0.;
                
                if(    ( bUpwards && TopLeftZ0.Y    < ITopY    && ITopY    < TopRightZ0.Y   ) // check for intersection with top
                    || (!bUpwards && BottomLeftZ0.Y < IBottomY && IBottomY < BottomRightZ0.Y) // check for intersection with bottom
                    )
                {
                    const FVector I = bUpwards ? FVector(CTop, ITopY, 0.) : FVector(CBottom, IBottomY, 0.);
                    const float Key = Spline->FindInputKeyClosestToWorldLocation(I);
                    const FVector EdgePoint = Spline->GetLocationAtSplineInputKey(Key, ESplineCoordinateSpace::World);
                    PC->ProjectWorldLocationToScreen
                        ( EdgePoint
                        , End
                        , true
                        );
                    PC->ProjectWorldLocationToScreen
                        ( EdgePoint + Spline->GetTangentAtSplineInputKey(Key, ESplineCoordinateSpace::World)
                        , ArriveTangentPos
                        , true
                        );
                    EndDir = ArriveTangentPos - End;
                    EndDir.Normalize();
                    EndDir = FVector2D::Zero();
                    
                    bDrawSpline = true;
                    
                    // debug
                    MakeCircle
                        ( OutDrawElements
                        , LayerId
                        , PG
                        , End / ViewportScale
                        , 50.
                        , 2
                        , ESlateDrawEffect::None
                        , FLinearColor::Red
                        );
                }
                else
                {
                    const bool bRightbound = ThisPoint.LeaveTangent.Y > 0.;
                    const double P1 = Points[i].OutVal.X;
                    const double P2 = Points[i].OutVal.Y;
                    const double T1 = Points[i].LeaveTangent.X;
                    const double T2 = Points[i].LeaveTangent.Y;
                    const double Q1 = bRightbound ? TopRightZ0.X : TopLeftZ0.X;
                    const double Q2 = bRightbound ? TopRightZ0.Y : TopLeftZ0.Y;
                    const FVector2D VecD = bRightbound ? BottomRightZ0 - TopRightZ0 : BottomLeftZ0 - TopLeftZ0;
                    const double D1 = VecD.X;
                    const double D2 = VecD.Y;
                    const double Alpha = (D1 * (P2 - Q2) - D2 * (P1 - Q1)) / (T1 * D2 - T2 * D1);
                    const FVector I = Points[i].OutVal + Alpha * Points[i].LeaveTangent;
                    const float Key = Spline->FindInputKeyClosestToWorldLocation(I);
                    const FVector EdgePoint = Spline->GetLocationAtSplineInputKey(Key, ESplineCoordinateSpace::World);
                    PC->ProjectWorldLocationToScreen
                        ( EdgePoint
                        , End
                        , true
                        );
                    PC->ProjectWorldLocationToScreen
                        ( EdgePoint + Spline->GetTangentAtSplineInputKey(Key, ESplineCoordinateSpace::World)
                        , ArriveTangentPos
                        , true
                        );
                    EndDir = (ArriveTangentPos - End);
                    EndDir.Normalize();
                    EndDir = FVector2D::Zero();
                    
                    bDrawSpline = true;
                    // debug
                    MakeCircle
                        ( OutDrawElements
                        , LayerId
                        , PG
                        , End / ViewportScale
                        , 50.
                        , 2
                        , ESlateDrawEffect::None
                        , FLinearColor::Yellow
                        );
                }
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
                else
                {
                    bDrawSpline = false;
                }
            }
        }
        
        if(bDrawSpline)
        {
            FSlateDrawElement::MakeSpline
                ( OutDrawElements
                , LayerId
                , PG
                , Start / ViewportScale
                , StartDir / ViewportScale
                , End / ViewportScale
                , EndDir / ViewportScale
                , 1
                , ESlateDrawEffect::None
                , FGColor
                );
        }
    }
}
