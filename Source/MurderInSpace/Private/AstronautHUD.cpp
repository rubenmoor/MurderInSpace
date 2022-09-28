// Fill out your copyright notice in the Description page of Project Settings.


#include "AstronautHUD.h"

#include "CharacterInSpace.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/Overlay.h"
#include "Kismet/GameplayStatics.h"
#include "UserWidgetHUDBorder.h"
#include "Components/Image.h"
#include "MyGameState.h"

#define LOCTEXT_NAMESPACE "mynamespace"

AAstronautHUD::AAstronautHUD()
{
	PrimaryActorTick.bCanEverTick = true;

	//HUDGreen = FLinearColor(FVector4d(0.263, 1., 0, 0.7));
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
		return;
	}
	
	UMG_AstronautHUD = CreateWidget<UUserWidget>(GetOwningPlayerController(), AssetUMG_AstronautHUD);
	UMG_AstronautHUD->AddToViewport();
	
	UserWidgetHUDBorder = FindOrFail<UUserWidgetHUDBorder>("BP_UserWidgetHUDBorder");
	UserWidgetHUDBorder->SetParams(X0, Y0, X1, Y1);

	TextVelocitySI = FindOrFail<UTextBlock>("TextVelocitySI");
	TextVelocityVCircle = FindOrFail<UTextBlock>("TextVelocityVCircle");
	TextVelocityDirection = FindOrFail<UTextBlock>("TextVelocityDirection");

	CanvasCenterOfMass = FindOrFail<UCanvasPanel>("CanvasCenterOfMass");
	
	// Overlay with two images
	OverlayCenterOfMass = FindOrFail<UOverlay>("OverlayCenterOfMass");
	ImgPointer = FindOrFail<UImage>("ImgPointer");

	const auto PC = GetOwningPlayerController();
	if(!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("AAstronautHUD::BeginPlay: no player controller, disabling tick; MyHUDWidget uses default values"))
		SetActorTickEnabled(false);
	}
	else
	{
		MyCharacter = PC->GetPawn<ACharacterInSpace>();
		if(!MyCharacter)
		{
			UE_LOG(LogTemp, Warning, TEXT("AAstronautHUD::BeginPlay: no pawn, disabling tick; MyHUDWidget uses default values"))
			SetActorTickEnabled(false);
		}
	}
}

void AAstronautHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(!UserWidgetHUDBorder || !TextVelocitySI || !TextVelocityVCircle || !TextVelocityDirection || !CanvasCenterOfMass || !OverlayCenterOfMass
		|| !ImgPointer) {
			UE_LOG(LogTemp, Error, TEXT("AAstronautHUD::Tick: Some widgets could not be found. Not doing anything."))
			return;
	}

	const float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(GetWorld());
	const TObjectPtr<AMyGameState> GS = GetWorld()->GetGameState<AMyGameState>();
	const FSpaceParams SP = GS->GetSpaceParams();
	const TObjectPtr<UOrbitComponent> Orbit = MyCharacter->GetOrbitComponent();
	const float Velocity = Orbit->GetVelocity();

	TextVelocitySI->SetText(FText::AsNumber(Velocity * SP.ScaleFactor, &FormattingOptions));
	TextVelocityVCircle->SetText(FText::AsNumber(Velocity / Orbit->GetCircleVelocity(SP.Alpha, SP.VecF1), &FormattingOptions));

	const float Angle = FQuat::FindBetween(FVector(1, 0, 0), Orbit->GetVecVelocity()).GetNormalized().GetTwistAngle(FVector(0, 0, 1));
	TextVelocityDirection->SetRenderTransformAngle(Angle * 180. / PI);

	const auto Vec2DSize = UWidgetLayoutLibrary::GetViewportSize(GetWorld());
	FVector2D ScreenLocation;
	
	// try to project to screen coordinates, ...
	const bool bProjected = GetOwningPlayerController()->ProjectWorldLocationToScreen(SP.VecF1, ScreenLocation);
	//const auto bProjected = UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(GetOwningPlayerController(), GI->VecF1, ScreenLocation, false);

	float XFromCenter;
	float YFromCenter;
	// ... which doesn't always work, i.e. F1 must be in front of the camera, I believe, which isn't always the case
	if(!bProjected)
	{
		FVector CameraLocation;
		FRotator CameraRotation;
		GetOwningPlayerController()->GetPlayerViewPoint(CameraLocation, CameraRotation);
		const FVector VecF1InViewportPlane =
			SP.VecF1 + CameraRotation.Vector() * (MyCharacter->GetOrbitComponent()->GetVecR() - SP.VecF1).Length();
		// ... but in that case we can help with a manual projection
		if(!GetOwningPlayerController()->ProjectWorldLocationToScreen(VecF1InViewportPlane, ScreenLocation))
		{
			// If the manual projection fails, too, we're out of options
			UE_LOG(LogActor, Error, TEXT("AAStronautHUD::Tick: couldn't project Center of mass to screen"))
		}
		// only we have to make sure that this manually conceived screen location doesn't accidentally
		// end up on screen
		XFromCenter = ScreenLocation.X / Vec2DSize.X - .5;
		YFromCenter = ScreenLocation.Y / Vec2DSize.Y - .5;
		YFromCenter += copysign(0.5 * YFromCenter / XFromCenter, YFromCenter);
		XFromCenter += copysign(0.5, XFromCenter);
	}
	else
	{
		XFromCenter = ScreenLocation.X / Vec2DSize.X - .5;
		YFromCenter = ScreenLocation.Y / Vec2DSize.Y - .5;
	}
	
	auto T = [this](float Y) -> float
	{
		const float C = 3. * Y1 - 1;
		
		// [-.5, .5] -> [0, 1]
		const float YNorm = Y + .5;
		
		return YNorm * (-2. * C * pow(YNorm, 2) + 3. * C * YNorm - C + 1);
	};
	auto XArc = [this, T] (float Y) -> float
	{
		const float TParam = T(Y);
		constexpr float X0Correction = 0.002;
		constexpr float X1Correction = -0.005;
		return X0 - X0Correction + 3. * (X1 - X1Correction - (X0 - X0Correction)) * TParam * (1. - TParam);
		// could be simplified to:
		// return X0 + 3. * (X1 - X0) * (Y + 0.5) * (0.5 - Y);
		// when Y1 is fixed to 1/3
	};
	 
	// off-screen
	if(abs(YFromCenter) > 0.5 || abs(XFromCenter) > 0.5 - XArc(YFromCenter))
	{
		CanvasCenterOfMass->SetVisibility(ESlateVisibility::Visible);
		
		float OverlayX, OverlayY;

		const auto Slot = UWidgetLayoutLibrary::SlotAsCanvasSlot(CanvasCenterOfMass);

		if(abs(YFromCenter) > abs(XFromCenter) / (1. - 2 * XArc(-0.5)))
		{
			// vertical case
			OverlayX = XFromCenter * 0.5 / abs(YFromCenter);
			if(YFromCenter < 0)
			{
				// above the viewport
				OverlayY = -0.5;
				Slot->SetAlignment(FVector2D(OverlayX / (1. - 2. * XArc(-0.5)) + .5, 0.));
			}
			else
			{
				// below the viewport
				OverlayY = 0.5;
				Slot->SetAlignment(FVector2D(OverlayX / (1. - 2. * XArc(-0.5)) + .5, 1.));
			}
		}
		else
		{
			// horizontal case
			OverlayY = YFromCenter * (0.5 - XArc(-0.5)) / abs(XFromCenter);
			if(XFromCenter < 0)
			{
				// to the left of the viewport
				OverlayX = -0.5 + XArc(OverlayY);
				Slot->SetAlignment(FVector2D(0., OverlayY + .5));
			}
			else
			{
				// to the right of the viewport
				OverlayX = 0.5 - XArc(OverlayY);
				Slot->SetAlignment(FVector2D(1., OverlayY + .5));
			}
		}
		
		UWidgetLayoutLibrary::SlotAsCanvasSlot(CanvasCenterOfMass)->SetPosition(FVector2D((OverlayX + .5) * Vec2DSize.X, (OverlayY + .5) * Vec2DSize.Y) / ViewportScale);
		//TextCenterOfMass->SetText(FText::Format(LOCTEXT("foo", "Y: {0}, YNorm: {1}, T(YNorm): {2}, XArc(YNorm): {3}"), OverlayY, (OverlayY + HalfHeight) / static_cast<float>(Vec2DSize.Y), T(OverlayY), XArc(OverlayY)));
		//TextCenterOfMass->SetText(FText::Format(LOCTEXT("foo", "{0}"), ViewportScale));
		ImgPointer->SetRenderTransformAngle(atan2(YFromCenter, XFromCenter) * 180. / PI + 135);
	}
	// on-screen
	else
	{
		//OverlayCenterOfMass->SetVisibility(ESlateVisibility::Collapsed);
		// TODO: paint green circle around center-of-mass
		UWidgetLayoutLibrary::SlotAsCanvasSlot(CanvasCenterOfMass)->SetPosition(ScreenLocation / ViewportScale);
		const TObjectPtr<UCanvasPanelSlot> Slot = UWidgetLayoutLibrary::SlotAsCanvasSlot(OverlayCenterOfMass);
		Slot->SetAlignment(FVector2D(.5, .5));
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
		return nullptr;
	}
}

#undef LOCTEXT_NAMESPACE