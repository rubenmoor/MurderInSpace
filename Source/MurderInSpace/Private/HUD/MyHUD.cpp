// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/MyHUD.h"

#include "Actors/OrbitComponent.h"
#include "Lib/UStateLib.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Actors/CharacterInSpace.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/Overlay.h"
#include "HUD/MyCommonButton.h"

void AMyHUD::InGameMenuShow()
{
	HideViewportParentWidgets();
	WidgetMenuInGame->SetVisibility(ESlateVisibility::Visible);
}

void AMyHUD::InGameMenuHide()
{
	HideViewportParentWidgets();
	WidgetHUD->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void AMyHUD::BeginPlay()
{
	Super::BeginPlay();

	const TObjectPtr<APlayerController> PC = GetOwningPlayerController();
	if(!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: BeginPlay: no player controller, disabling tick"), *GetFullName())
		SetActorTickEnabled(false);
		return;
	}
	// get playing character
	MyCharacter = PC->GetPawn<ACharacterInSpace>();
	if(!MyCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: BeginPlay: no pawn, disabling tick"), *GetFullName())
		SetActorTickEnabled(false);
		return;
	}

	// set up HUD

	if(!WidgetHUDClass)
	{
		UE_LOG(LogSlate, Error, TEXT("%s: WidgetHUDClass null"), *GetFullName())
		return;
	}

	WidgetHUD = CreateWidget(GetWorld(), WidgetHUDClass, FName(TEXT("HUD")));
	WidgetHUD->SetVisibility(ESlateVisibility::HitTestInvisible);
	WidgetHUD->AddToViewport(0);
	
	TextVelocitySI        = FindOrFail<UTextBlock  >(WidgetHUD, FName(TEXT("TextVelocitySI"       )));
	TextVelocityVCircle   = FindOrFail<UTextBlock  >(WidgetHUD, FName(TEXT("TextVelocityVCircle"  )));
	TextVelocityDirection = FindOrFail<UTextBlock  >(WidgetHUD, FName(TEXT("TextVelocityDirection")));
	CanvasCenterOfMass    = FindOrFail<UCanvasPanel>(WidgetHUD, FName(TEXT("CanvasCenterOfMass"   )));
	
	// Overlay with two images
	OverlayCenterOfMass   = FindOrFail<UOverlay    >(WidgetHUD, FName(TEXT("OverlayCenterOfMass"  )));
	ImgPointer            = FindOrFail<UImage      >(WidgetHUD, FName(TEXT("ImgPointer"           )));
	
	if  (  !TextVelocitySI
		|| !TextVelocityVCircle
		|| !TextVelocityDirection
		|| !CanvasCenterOfMass
		|| !OverlayCenterOfMass
		|| !ImgPointer
		)
	{
		UE_LOG
			( LogTemp
			, Error
			, TEXT("%s: Some widgets could not be found. Disabling Tick.")
			, *GetFullName()
			)
		SetActorTickEnabled(false);
	}
	
	// set up in-game menu
	
	if(!WidgetMenuInGameClass)
	{
		UE_LOG(LogSlate, Error, TEXT("%s: UMGWidgetInGameClass null"), *GetFullName())
		return;
	}
	WidgetMenuInGame = CreateWidget(GetWorld(), WidgetMenuInGameClass, FName(TEXT("In-Game Menu")));
	WidgetMenuInGame->SetVisibility(ESlateVisibility::Collapsed);
	WidgetMenuInGame->AddToViewport(1);

	const TObjectPtr<UMyGameInstance> GI = GetGameInstance<UMyGameInstance>();
	
	WithWidget<UMyCommonButton>(WidgetMenuInGame, FName(TEXT("BtnResume")), [GI] (TObjectPtr<UMyCommonButton> Button)
	{
		Button->OnClicked().AddLambda([GI] () { GI->GotoInGame(); });
	});
	WithWidget<UMyCommonButton>(WidgetMenuInGame, FName(TEXT("BtnLeave")), [GI] (TObjectPtr<UMyCommonButton> Button)
	{
		Button->OnClicked().AddLambda([GI] () { GI->GotoInMenuMain(); });
	});
	WithWidget<UMyCommonButton>(WidgetMenuInGame, FName(TEXT("BtnQuit")), [GI] (TObjectPtr<UMyCommonButton> Button)
	{
		Button->OnClicked().AddLambda([GI] () { GI->QuitGame(); });
	});
}

void AMyHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	const TObjectPtr<APlayerController> PC = GetOwningPlayerController();

	const float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(GetWorld());
	const FPhysics Physics = UStateLib::GetPhysicsUnsafe(this);
	const TObjectPtr<UOrbitComponent> Orbit = MyCharacter->GetOrbitComponent();
	const float Velocity = Orbit->GetVelocity();

	TextVelocitySI->SetText(FText::AsNumber(Velocity * Physics.ScaleFactor, &FormattingOptions));
	TextVelocityVCircle->SetText(
		FText::AsNumber(
			Velocity / Orbit->GetCircleVelocity(Physics.Alpha, Physics.VecF1
		), &FormattingOptions));

	const float Angle =
		FQuat::FindBetween
			( FVector(1, 0, 0)
			, Orbit->GetVecVelocity()).GetNormalized().GetTwistAngle(FVector(0, 0, 1)
			);
	TextVelocityDirection->SetRenderTransformAngle(Angle * 180. / PI);

	const auto Vec2DSize = UWidgetLayoutLibrary::GetViewportSize(GetWorld());
	FVector2D ScreenLocation;
	
	// try to project to screen coordinates, ...
	const bool bProjected = PC->ProjectWorldLocationToScreen(Physics.VecF1, ScreenLocation);
	//const auto bProjected = UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(GetOwningPlayerController(), GI->VecF1, ScreenLocation, false);

	float XFromCenter;
	float YFromCenter;
	// ... which doesn't always work, i.e. F1 must be in front of the camera, I believe, which isn't always the case
	if(!bProjected)
	{
		FVector CameraLocation;
		FRotator CameraRotation;
		PC->GetPlayerViewPoint(CameraLocation, CameraRotation);
		const FVector VecF1InViewportPlane =
			  Physics.VecF1
			+ CameraRotation.Vector()
			* (MyCharacter->GetOrbitComponent()->GetVecR() - Physics.VecF1).Length();
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
		
		UWidgetLayoutLibrary::SlotAsCanvasSlot(CanvasCenterOfMass)->SetPosition(
			  FVector2D((OverlayX + .5) * Vec2DSize.X, (OverlayY + .5) * Vec2DSize.Y)
			/ ViewportScale);
		//TextCenterOfMass->SetText(FText::Format(LOCTEXT("foo", "Y: {0}, YNorm: {1}, T(YNorm): {2}, XArc(YNorm): {3}"), OverlayY, (OverlayY + HalfHeight) / static_cast<float>(Vec2DSize.Y), T(OverlayY), XArc(OverlayY)));
		//TextCenterOfMass->SetText(FText::Format(LOCTEXT("foo", "{0}"), ViewportScale));
		ImgPointer->SetRenderTransformAngle(atan2(YFromCenter, XFromCenter) * 180. / PI + 135);
	}
	// on-screen
	else
	{
		OverlayCenterOfMass->SetVisibility(ESlateVisibility::Collapsed);
		//UWidgetLayoutLibrary::SlotAsCanvasSlot(CanvasCenterOfMass)->SetPosition(ScreenLocation / ViewportScale);
		//const TObjectPtr<UCanvasPanelSlot> Slot = UWidgetLayoutLibrary::SlotAsCanvasSlot(OverlayCenterOfMass);
		//Slot->SetAlignment(FVector2D(.5, .5));
		
		// TODO: paint green circle around center-of-mass
	}
}
