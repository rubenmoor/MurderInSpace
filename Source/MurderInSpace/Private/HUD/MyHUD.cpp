// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/MyHUD.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Actors/MyCharacter.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h"
#include "HUD/UW_HUD.h"
#include "HUD/UW_MenuInGame.h"
#include "Modes/MyPlayerController.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

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

void AMyHUD::SetReadyFlags(EHUDReady ReadyFlags)
{
	HUDReady |= ReadyFlags;
	if(HUDReady == EHUDReady::All)
	{
		UE_LOG
			( LogMyGame
			, Display
			, TEXT("%s: HUD ready")
			, *GetFullName()
			)
		SetActorTickEnabled(true);
	}
	else
	{
		UE_LOG
			( LogMyGame
			, Display
			, TEXT("%s: Internal %s, Orbit %s, Pawn->GetOrbit %s")
			, *GetFullName()
			, !(HUDReady & EHUDReady::InternalReady ) ? TEXT("waiting") : TEXT("ready")
			, !(HUDReady & EHUDReady::OrbitReady) ? TEXT("waiting") : TEXT("ready")
			, !(HUDReady & EHUDReady::PawnOrbitReady    ) ? TEXT("waiting") : TEXT("ready")
			)
	}
}

void AMyHUD::BeginPlay()
{
	Super::BeginPlay();

#if WITH_EDITOR
	if(GEditor->IsSimulateInEditorInProgress())
	{
		return;
	}
#endif
	
	bool bHasProblems = false;
	
	const AMyPlayerController* PC = Cast<AMyPlayerController>(GetOwningPlayerController());
	if(!PC)
	{
		UE_LOG(LogMyGame, Warning, TEXT("%s: BeginPlay: no player controller"), *GetFullName())
		bHasProblems = true;
	}
	
	// get playing character
	
	MyCharacter = Cast<AMyCharacter>(GetOwningPawn());
	if(!IsValid(MyCharacter))
	{
		UE_LOG(LogMyGame, Warning, TEXT("%s: BeginPlay: no pawn"), *GetFullName())
		bHasProblems = true;
	}
	// at BeginPlay, the orbit hasn't replicated to the client yet,
	//else if(MyCharacter->Children.IsEmpty())
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("%s: BeginPlay: orbit actor null"), *GetFullName())
	//}

	UGameInstance* GI = GetGameInstance();
	
	// set up HUD

	if(IsValid(WidgetHUDClass))
	{
		WidgetHUD = CreateWidget<UUW_HUD>(GI, WidgetHUDClass, "HUD");
		WidgetHUD->SetVisibility(ESlateVisibility::HitTestInvisible);
		WidgetHUD->AddToViewport(0);
	}
	else
	{
		UE_LOG(LogMyGame, Error, TEXT("%s: WidgetHUDClass null"), *GetFullName())
		bHasProblems = true;
	}
	// set up in-game menu
	
	if(IsValid(WidgetMenuInGameClass))
	{
		WidgetMenuInGame = CreateWidget<UUW_MenuInGame>(GI, WidgetMenuInGameClass, "In-Game Menu");
		WidgetMenuInGame->SetVisibility(ESlateVisibility::Collapsed);
		WidgetMenuInGame->AddToViewport(1);
	}
	else
	{
		UE_LOG(LogMyGame, Error, TEXT("%s: WidgetInGameClass null"), *GetFullName())
		bHasProblems = true;
	}

	if(!bHasProblems)
	{
		if(MyCharacter->GetLocalRole() == ROLE_Authority)
		{
			SetActorTickEnabled(true);
		}
		else
		{
			SetReadyFlags(EHUDReady::InternalReady);
		}
	}
	else
	{
		UE_LOG(LogMyGame, Error, TEXT("%s: there were problems, not ticking"), *GetFullName())
	}
}

void AMyHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	const APlayerController* PC = GetOwningPlayerController();

	const FPhysics Physics = GEngine->GetEngineSubsystem<UMyState>()->GetPhysicsAny(this);
	AOrbit* Orbit = MyCharacter->GetOrbit();
	const float Velocity = Orbit->GetScalarVelocity();

	extern ENGINE_API float GAverageFPS;
	WidgetHUD->TextFPS->SetText(FText::AsNumber(GAverageFPS, &FOFPS));
	WidgetHUD->TextVelocitySI->SetText(FText::AsNumber(Velocity * Physics.ScaleFactor, &FOVelocity));
	WidgetHUD->TextVelocityVCircle->SetText(
		FText::AsNumber(Velocity / Orbit->GetCircleVelocity(Physics).Length(), &FOVelocity)
		);
	WidgetHUD->TextCameraHeight->SetText(
		FText::AsNumber(MyCharacter->GetSpringArmLength() * Physics.ScaleFactor, &FODistance)
		);
	
	const float DistanceF1 = Orbit->GetVecRKepler(Physics).Length() * Physics.ScaleFactor;
	
	const float AngleVelocityArrow =
		FQuat::FindBetween
			( FVector(1, 0, 0)
			, Orbit->GetVecVelocity()).GetNormalized().GetTwistAngle(FVector(0, 0, 1)
			);
	WidgetHUD->TextVelocityDirection->SetRenderTransformAngle(AngleVelocityArrow * 180. / PI);

	FVector2D ScreenLocation;
	
	// try to project to screen coordinates, ...
	const bool bProjected = PC->ProjectWorldLocationToScreen(Physics.VecF1, ScreenLocation);
	//const auto bProjected = UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(GetOwningPlayerController(), GI->VecF1, ScreenLocation, false);

	FVector2D Pos;
	// ... which doesn't always work, i.e. F1 must be in front of the camera, I believe, which isn't always the case
	if(!bProjected)
	{
		FVector CameraLocation;
		FRotator CameraRotation;
		PC->GetPlayerViewPoint(CameraLocation, CameraRotation);
		const FVector VecF1InViewportPlane =
			  Physics.VecF1
			+ CameraRotation.Vector()
			* Orbit->GetVecRKepler(Physics).Length();
		// ... but in that case we can help with a manual projection
		if(!PC->ProjectWorldLocationToScreen(VecF1InViewportPlane, ScreenLocation))
		{
			// If the manual projection fails, too, we're out of options
			UE_LOG(LogMyGame, Error, TEXT("AMyHUD::Tick: couldn't project Center of mass to screen"))
		}
		// only we have to make sure that this manually conceived screen location doesn't accidentally
		// end up on screen
		Pos = ScreenToCenter(this, ScreenLocation);
		Pos.Y += copysign(0.5 * Pos.Y / Pos.X, Pos.Y);
		Pos.X += copysign(0.5, Pos.X);
	}
	else
	{
		Pos = ScreenToCenter(this, ScreenLocation);
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
	if(abs(Pos.Y) > 0.5 || abs(Pos.X) > 0.5 - XArc(Pos.Y))
	{
		if(bF1OnScreen)
		{
			WidgetHUD->F1MarkerHide();
			WidgetHUD->CanvasCenterOfMass->SetVisibility(ESlateVisibility::Visible);
			bF1OnScreen = false;
		}
		
		WidgetHUD->TextDistanceF1OffScreen->SetText(FText::AsNumber(DistanceF1, &FODistance));
		
		float OverlayX, OverlayY;

		const auto SlotCenterOfMass =
			UWidgetLayoutLibrary::SlotAsCanvasSlot(WidgetHUD->CanvasCenterOfMass);
		FVector2D Vec2Alignment;

		if(abs(Pos.Y) > abs(Pos.X) / (1. - 2 * XArc(-0.5)))
		{
			// vertical case
			OverlayX = Pos.X * 0.5 / abs(Pos.Y);
			if(Pos.Y < 0)
			{
				// above the viewport
				OverlayY = -0.5;
				Vec2Alignment = { OverlayX / (1. - 2. * XArc(-0.5)) + .5, 0. };
			}
			else
			{
				// below the viewport
				OverlayY = 0.5;
				Vec2Alignment = { OverlayX / (1. - 2. * XArc(-0.5)) + .5, 1. };
			}
		}
		else
		{
			// horizontal case
			OverlayY = Pos.Y * (0.5 - XArc(-0.5)) / abs(Pos.X);
			if(Pos.X < 0)
			{
				// to the left of the viewport
				OverlayX = -0.5 + XArc(OverlayY);
				Vec2Alignment = {0., OverlayY + .5 };
			}
			else
			{
				// to the right of the viewport
				OverlayX = 0.5 - XArc(OverlayY);
				Vec2Alignment = {1., OverlayY + .5 };
			}
		}

		SlotCenterOfMass->SetAlignment(Vec2Alignment);
		SlotCenterOfMass->SetPosition
			( CenterToScreenScaled(this, { OverlayX, OverlayY } )
			);
		const float AngleF1Marker = atan2(Pos.Y, Pos.X) + PI;
		WidgetHUD->ImgPointer->SetRenderTransformAngle(AngleF1Marker * 180. / PI - 45);
		const auto SlotDistanceF1 = UWidgetLayoutLibrary::SlotAsCanvasSlot(WidgetHUD->HoriDistanceF1OffScreen);
		SlotDistanceF1->SetPosition(FVector2D(cos(AngleF1Marker), sin(AngleF1Marker)) * DistanceF1Radius);
		SlotDistanceF1->SetAlignment(Vec2Alignment);
	}
	// on-screen
	else
	{
		if(!bF1OnScreen)
		{
			WidgetHUD->CanvasCenterOfMass->SetVisibility(ESlateVisibility::Collapsed);
			WidgetHUD->F1MarkerShow();
			bF1OnScreen = true;
		}
		
		// TODO: show distance to F1
		WidgetHUD->TextDistanceF1OnScreen->SetText(FText::AsNumber(DistanceF1, &FODistance));
		FVector2D Vec2Alignment = FVector2D(0., 0.);
		WidgetHUD->F1Marker.Coords = CenterToScreenScaled(this, Pos);
		const auto SlotDistanceF1 = UWidgetLayoutLibrary::SlotAsCanvasSlot(WidgetHUD->HoriDistanceF1OnScreen);
		const float AngleF1Marker = atan2(Pos.Y, Pos.X) + PI;
		SlotDistanceF1->SetPosition(FVector2D(cos(AngleF1Marker), sin(AngleF1Marker)) * DistanceF1Radius);
		SlotDistanceF1->SetAlignment(Vec2Alignment);
	}
}
