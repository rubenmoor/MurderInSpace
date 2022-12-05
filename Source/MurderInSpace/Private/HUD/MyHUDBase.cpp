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

	FormattingOptions.SetUseGrouping(false);
	FormattingOptions.SetMinimumFractionalDigits(1);
	FormattingOptions.SetMaximumFractionalDigits(1);

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
	UWorld* World = Outer->GetWorld();
	const FVector2D Vec2DSize = UWidgetLayoutLibrary::GetViewportSize(World);
	const float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(World);
	return
		{ (CenterCoords.X + 0.5) * Vec2DSize.X / ViewportScale
		, (CenterCoords.Y + 0.5) *Vec2DSize.Y / ViewportScale
		};
}

void AMyHUDBase::BeginPlay()
{
	Super::BeginPlay();

	LocalPlayerContext = FLocalPlayerContext(GetOwningPlayerController());
	
	UGameInstance* GI = GetGameInstance();
	
	WidgetHUDBorder = CreateWidget<UWidgetHUDBorder>(GI, WidgetHUDBorderClass, "HUD Border");
	// TODO: proly have to set foreground color
	WidgetHUDBorder->SetParams(X0, Y0, X1, Y1);
	WidgetHUDBorder->AddToViewport(-1);
}

void AMyHUDBase::HideViewportParentWidgets()
{
	if(IsValid(this))
	{
		TArray<UUserWidget*> ParentWidgets;
		MyObjectIterator<UUserWidget> IWidget([this] (const UUserWidget* Widget)
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
