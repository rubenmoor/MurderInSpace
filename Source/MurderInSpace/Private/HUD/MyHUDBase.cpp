// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/MyHUDBase.h"

#include "Actors/MyCharacter.h"
#include "Blueprint/UserWidget.h"
#include "HUD/WidgetHUDBorder.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Lib/FunctionLib.h"

AMyHUDBase::AMyHUDBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	FOVelocity.SetUseGrouping(false);
	FOVelocity.SetMinimumFractionalDigits(1);
	FOVelocity.SetMaximumFractionalDigits(1);

	FOFPS.SetUseGrouping(false);
	FOFPS.SetMaximumFractionalDigits(0);

	FODistance.SetUseGrouping(false);
	FODistance.SetMaximumFractionalDigits(0);

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
				&& !Widget->IsA(UWidgetHUDBorder::StaticClass());
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
