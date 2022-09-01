// Fill out your copyright notice in the Description page of Project Settings.


#include "AstronautHUD.h"

#include <algorithm>

#include "CharacterInSpace.h"
#include "MyGameInstance.h"
#include "MyHUDWidget.h"
#include "OrbitDataComponent.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"

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
	TextCenterOfMass = FindOrFail<UTextBlock>("TextCenterOfMass");

	ImgDebug = FindOrFail<UImage>("ImgDebug");
	
	const auto PC = GetOwningPlayerController();
	if(!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("AAstronautHUD::BeginPlay: no player controller, disabling tick; MyHUDWidget uses default values"))
		SetActorTickEnabled(false);
	}
	else
	{
		Pawn = PC->GetPawn<ACharacterInSpace>();
		if(!Pawn)
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
		|| !ImgPointer || !TextCenterOfMass || !ImgDebug) {
			UE_LOG(LogTemp, Error, TEXT("AAstronautHUD::Tick: Some widgets could not be found. Not doing anything."))
			return;
	}

	const auto ViewportScale = UWidgetLayoutLibrary::GetViewportScale(GetWorld());
	const auto GI = GetGameInstance<UMyGameInstance>();
	const auto OrbitData = Pawn->GetOrbitDataComponent();
	const auto Velocity = OrbitData->GetVelocity();

	TextVelocitySI->SetText(FText::AsNumber(Velocity * GI->ScaleFactor, &FormattingOptions));
	TextVelocityVCircle->SetText(FText::AsNumber(Velocity / OrbitData->GetCircleVelocity(GI->Alpha, GI->VecF1), &FormattingOptions));

	const auto Angle = FQuat::FindBetween(FVector(1, 0, 0), OrbitData->GetVecVelocity()).GetNormalized().GetTwistAngle(FVector(0, 0, 1));
	TextVelocityDirection->SetRenderTransformAngle(Angle * 180. / PI);

	FVector2D ScreenLocation;
	const auto bProjected = GetOwningPlayerController()->ProjectWorldLocationToScreen(GI->VecF1, ScreenLocation);
	if(!bProjected)
	{
		FVector CameraLocation;
		FRotator CameraRotation;
		GetOwningPlayerController()->GetPlayerViewPoint(CameraLocation, CameraRotation);
		const auto VecF1InViewportPlane = GI->VecF1 + CameraRotation.Vector() * (GetOwningPawn()->GetActorLocation() - GI->
			VecF1).Length();
		if(!GetOwningPlayerController()->ProjectWorldLocationToScreen(VecF1InViewportPlane, ScreenLocation))
		{
			UE_LOG(LogActor, Error, TEXT("AAStronautHUD::Tick: couldn't project Center of mass to screen"))
		}
	}
	
	const auto Vec2DSize = UWidgetLayoutLibrary::GetViewportSize(GetWorld());
	const float XFromCenter = ScreenLocation.X / Vec2DSize.X - .5;
	const float YFromCenter = ScreenLocation.Y / Vec2DSize.Y - .5;
	
	// off-screen
	if(ScreenLocation.X < 0 || ScreenLocation.X > Vec2DSize.X || ScreenLocation.Y < 0 || ScreenLocation.Y > Vec2DSize.Y)
	{
		CanvasCenterOfMass->SetVisibility(ESlateVisibility::Visible);
		const auto Slot = UWidgetLayoutLibrary::SlotAsCanvasSlot(OverlayCenterOfMass);
		
		const float C = 3. * Y1 - 1;
		auto T = [C](float Y) -> float
		{
			// [-.5, .5] -> [0, 1]
			const float YNorm = Y + .5;
			return YNorm * (-2. * C * pow(YNorm, 2) + 3. * C * YNorm - C + 1);
		};
		auto XArc = [this, T] (float Y) -> float
		{
			const auto TParam = T(Y);
			return X0 - 3. * X1 * TParam * (1. - TParam);
		};
		
		float OverlayX, OverlayY;

		// TODO: this ratio isn't correct
		// instead, I need the smallest square that fits into the HUD maybe
		if(abs(YFromCenter/XFromCenter) > Vec2DSize.Y / static_cast<float>(Vec2DSize.X))
		{
			const auto XMinMax = .5 - XArc(-.5);
			OverlayX = std::clamp<float>(XFromCenter * .5 / abs(YFromCenter), -XMinMax, XMinMax);
			
			// below the viewport
			if(YFromCenter > 0.)
			{
				OverlayY = .5;
				//Slot->SetAlignment(FVector2D(OverlayX + .5, 1));
			}
			
			// above the viewport
			else
			{
				OverlayY = -.5;
				//Slot->SetAlignment(FVector2D(OverlayX + .5, 0.));
			}
		}
		else
		{
			OverlayY = YFromCenter * .5 / abs(XFromCenter);

			// to the right of the viewport
			if(XFromCenter > 0.)
			{
				OverlayX = .5 - XArc(OverlayY);
				//Slot->SetAlignment(FVector2D(1., OverlayY + .5));
			}

			// to the left of the viewport
			else
			{
				OverlayX = -(.5 - XArc(OverlayY));
				//Slot->SetAlignment(FVector2D(0., OverlayY + .5));
			}
		}
		UWidgetLayoutLibrary::SlotAsCanvasSlot(CanvasCenterOfMass)->SetPosition(FVector2D((OverlayX + .5) * Vec2DSize.X, (OverlayY + .5) * Vec2DSize.Y) / ViewportScale);
		//TextCenterOfMass->SetText(FText::Format(LOCTEXT("foo", "Y: {0}, YNorm: {1}, T(YNorm): {2}, XArc(YNorm): {3}"), OverlayY, (OverlayY + HalfHeight) / static_cast<float>(Vec2DSize.Y), T(OverlayY), XArc(OverlayY)));
		//TextCenterOfMass->SetText(FText::Format(LOCTEXT("foo", "{0}"), ViewportScale));
	}
	// on-screen
	else
	{
		//OverlayCenterOfMass->SetVisibility(ESlateVisibility::Collapsed);
		// TODO: paint green circle around center-of-mass
		UWidgetLayoutLibrary::SlotAsCanvasSlot(CanvasCenterOfMass)->SetPosition(ScreenLocation / ViewportScale);
		const auto Slot = UWidgetLayoutLibrary::SlotAsCanvasSlot(OverlayCenterOfMass);
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