// Fill out your copyright notice in the Description page of Project Settings.


#include "AstronautHUD.h"

#include <algorithm>

#include "CharacterInSpace.h"
#include "MyGameInstance.h"
#include "OrbitDataComponent.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h"

AAstronautHUD::AAstronautHUD()
{
	PrimaryActorTick.bCanEverTick = true;
	
	FormattingOptions.SetUseGrouping(false);
	FormattingOptions.SetMinimumFractionalDigits(1);
	FormattingOptions.SetMaximumFractionalDigits(1);
}

void AAstronautHUD::BeginPlay()
{
	Super::BeginPlay();

	if(!AssetUMG_AstronautHUD)
	{
		RequestEngineExit("AAstronautHUD::BeginPlay: AssetUMG_AstronautHUD null");
	}
	
	UMG_AstronautHUD = CreateWidget<UUserWidget>(GetOwningPlayerController(), AssetUMG_AstronautHUD);
	UMG_AstronautHUD->AddToViewport();

	CanvasPanelMain = FindOrFail<UCanvasPanel>("CanvasPanelMain");
	TextVelocitySI = FindOrFail<UTextBlock>("TextVelocitySI");
	TextVelocityVCircle = FindOrFail<UTextBlock>("TextVelocityVCircle");
	TextVelocityDirection = FindOrFail<UTextBlock>("TextVelocityDirection");

	// Overlay with two images
	OverlayCenterOfMass = FindOrFail<UOverlay>("OverlayCenterOfMass");
	ImgPointer = FindOrFail<UImage>("ImgPointer");
}

void AAstronautHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const auto ViewportScale = UWidgetLayoutLibrary::GetViewportScale(GetWorld());
	const auto GI = GetGameInstance<UMyGameInstance>();
	const auto OrbitData = GetOwningPlayerController()->GetPawn<ACharacterInSpace>()->GetOrbitDataComponent();
	const auto Velocity = OrbitData->GetVelocity();

	TextVelocitySI->SetText(FText::AsNumber(Velocity * GI->ScaleFactor, &FormattingOptions));
	TextVelocityVCircle->SetText(FText::AsNumber(Velocity / OrbitData->GetCircleVelocity(GI->Alpha, GI->VecF1), &FormattingOptions));

	const auto Angle = FQuat::FindBetween(FVector(1, 0, 0), OrbitData->GetVecVelocity()).GetNormalized().GetTwistAngle(FVector(0, 0, 1));
	TextVelocityDirection->SetRenderTransformAngle(Angle * 180. / PI);

	FVector2D ScreenLocation;
	GetOwningPlayerController()->ProjectWorldLocationToScreen(GI->VecF1, ScreenLocation);
	//UE_LOG(LogTemp, Display, TEXT("%s"), *ScreenLocation.ToString())
	
	int32 SizeX, SizeY;
	GetOwningPlayerController()->GetViewportSize(SizeX, SizeY);

	//UE_LOG(LogTemp, Display, TEXT("Screen Location: %s"), *ScreenLocation.ToString())
	
	// off-screen
	if(ScreenLocation.X < 0 || ScreenLocation.X > SizeX || ScreenLocation.Y < 0 || ScreenLocation.Y > SizeY)
	{
		const auto HalfWidth = SizeX / 2.;
		const auto HalfHeight = SizeY / 2.;
		const auto XFromCenter = ScreenLocation.X - HalfWidth;
		const auto YFromCenter = ScreenLocation.Y - HalfHeight;
		float OverlayX, OverlayY;
		if(abs(YFromCenter/XFromCenter) > SizeY / SizeX)
		{
			OverlayX = HalfWidth + XFromCenter * HalfHeight / abs(YFromCenter);
			if(YFromCenter > 0.)
			{
				OverlayY = SizeY;
			}
			else
			{
				OverlayY = 0.;
			}
		}
		else
		{
			OverlayY = HalfHeight + YFromCenter * HalfWidth / abs(XFromCenter);
			if(XFromCenter > 0)
			{
				OverlayX = SizeX;
			}
			else
			{
				OverlayX = 0.;
			}
		}
		//const auto OverlayX = abs(YFromCenter) > HalfHeight ? std::clamp<float>(HalfWidth + XFromCenter * HalfHeight / YFromCenter, 0., SizeX) : HalfWidth + copysign(HalfWidth, XFromCenter);
		//const auto OverlayY = abs(XFromCenter) > HalfWidth ? std::clamp<float>(HalfHeight + YFromCenter * HalfWidth / XFromCenter, 0., SizeY) : HalfHeight + copysign(HalfHeight, YFromCenter);
		OverlayCenterOfMass->SetVisibility(ESlateVisibility::Visible);
		UWidgetLayoutLibrary::SlotAsCanvasSlot(OverlayCenterOfMass)->SetPosition(FVector2D(OverlayX, OverlayY) / ViewportScale);
	}
	// on-screen
	else
	{
		//OverlayCenterOfMass->SetVisibility(ESlateVisibility::Collapsed);
		// TODO: paint green circle around center-of-mass
		UWidgetLayoutLibrary::SlotAsCanvasSlot(OverlayCenterOfMass)->SetPosition(ScreenLocation / ViewportScale);
	}
}

template <typename WidgetT>
TObjectPtr<WidgetT> AAstronautHUD::FindOrFail(const FName& Name) const
{
	if(const auto Widget = UMG_AstronautHUD->WidgetTree.Get()->FindWidget<WidgetT>(Name))
	{
		return Widget;
	}
	else
	{
		UE_LOG(LogSlate, Error, TEXT("AAstronautHUD::FindOrFail: Couldn't find %s"), *Name.ToString())
		RequestEngineExit("HUD: error in widgets");
		return nullptr;
	}
}
