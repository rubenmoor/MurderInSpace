// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/MyHUDBase.h"

#include "Actors/MyCharacter.h"
#include "Blueprint/UserWidget.h"
#include "HUD/UW_WidgetHUDBorder.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Border.h"
#include "Components/CanvasPanelSlot.h"
#include "HUD/MyHUD.h"
#include "HUD/MyHUDMenu.h"
#include "Lib/FunctionLib.h"
#include "Menu/UW_MenuBackground.h"
#include "Menu/UW_Settings.h"

AMyHUDBase::AMyHUDBase()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

    FOVelocity = MakeShared<FNumberFormattingOptions>();
    FOVelocity->SetUseGrouping(false);
    FOVelocity->SetMinimumFractionalDigits(1);
    FOVelocity->SetMaximumFractionalDigits(1);

    FOFPS = MakeShared<FNumberFormattingOptions>();
    FOFPS->SetUseGrouping(false);
    FOFPS->SetMaximumFractionalDigits(0);

    FODistance = MakeShared<FNumberFormattingOptions>();
    FODistance->SetUseGrouping(false);
    FODistance->SetMaximumFractionalDigits(0);

    FODPIScale = MakeShared<FNumberFormattingOptions>();
    FODPIScale->SetUseGrouping(false);
    FODPIScale->SetMaximumFractionalDigits(1);
    FODPIScale->SetMinimumFractionalDigits(1);

    // TODO: find a way to set HUD default style
}

FVector2D AMyHUDBase::ScreenToCenter(const UObject* Outer, FVector2D ScreenCoords)
{
    const FVector2D Vec2DSize = UWidgetLayoutLibrary::GetViewportSize(Outer->GetWorld());
    return
        { ScreenCoords.X / Vec2DSize.X - 0.5
        , ScreenCoords.Y / Vec2DSize.Y - 0.5
        };
}

FVector2D AMyHUDBase::CenterToScreenScaled(const UObject* Outer, FVector2D CenterCoords)
{
    const float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(Outer->GetWorld());
    return CenterToScreen(Outer, CenterCoords) / ViewportScale;
}

FVector2D AMyHUDBase::CenterToScreen(const UObject* Outer, FVector2D CenterCoords)
{
    const FVector2D Vec2DSize = UWidgetLayoutLibrary::GetViewportSize(Outer->GetWorld());
    return
        { (CenterCoords.X + 0.5) * Vec2DSize.X
        , (CenterCoords.Y + 0.5) *Vec2DSize.Y
        };
}

void AMyHUDBase::BeginPlay()
{
    Super::BeginPlay();

    LocalPlayerContext = FLocalPlayerContext(GetOwningPlayerController());
    
    UGameInstance* GI = GetGameInstance();
    
    if(IsValid(WidgetSettingsClass))
    {
        WidgetSettings = CreateWidget<UUW_Settings>(GI, WidgetSettingsClass, "Settings");
        WidgetSettings->SetVisibility(ESlateVisibility::Collapsed);
        WidgetSettings->AddToViewport(2);
    }
    else
    {
        UE_LOG(LogMyGame, Error, TEXT("%s: WidgetSettingsClass null"), *GetFullName())
    }
    
    if(IsValid(WidgetBackgroundClass))
    {
        WidgetBackground = CreateWidget<UUW_MenuBackground>(GI, WidgetBackgroundClass, "Background");
        if(Cast<AMyHUDMenu>(this))
        {
            WidgetBackground->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            WidgetBackground->SetVisibility(ESlateVisibility::Collapsed);
        }
        WidgetBackground->AddToViewport(1);
    }
    else
    {
        UE_LOG(LogMyGame, Error, TEXT("%s: WidgetBackgroundClass null"), *GetFullName())
    }
}

void AMyHUDBase::HideViewportParentWidgets()
{
    if(IsValid(this))
    {
        TArray<UUserWidget*> ParentWidgets;
        TMyObjectIterator<UUserWidget> IWidget([this] (const UUserWidget* Widget)
        {
            return Widget->GetWorld() == GetWorld()
                && Widget->IsInViewport()
                && !Widget->IsA(UUW_WidgetHUDBorder::StaticClass())
                && !Widget->IsA(UUW_MenuBackground::StaticClass());
        });
        for(; IWidget; ++IWidget)
        {
            (*IWidget)->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
    else
    {
        UE_LOG(LogSlate, Warning, TEXT("AMyHUDBase: invalid HUD"))
    }
}
void AMyHUDBase::SettingsShow()
{
    HideViewportParentWidgets();
    WidgetSettings->SetVisibility(ESlateVisibility::Visible);
}

void AMyHUDBase::ScaleBorderWidget(float DPIScale)
{
    FVector2D New = MenuBackgroundSize / DPIScale;
	const auto SlotBorder = UWidgetLayoutLibrary::SlotAsCanvasSlot(WidgetBackground->GetBorder());
	FMargin Margin = SlotBorder->GetOffsets();
	Margin.Right = New.X;
	Margin.Bottom = New.Y;
	SlotBorder->SetOffsets(Margin);
}
