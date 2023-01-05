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

void AMyHUD::MarkOrbitInitDone()
{
	bOrbitNetInitDone = true;
	if(bSuccessfulInitialization)
	{
		SetActorTickEnabled(true);
		UE_LOG(LogMyGame, Display, TEXT("%s: MarkOrbitInitDone: initialization successful, tick enabled"), *GetFullName())
	}
	else
	{
		UE_LOG(LogMyGame, Error, TEXT("%s: waiting for BeginPlay"), *GetFullName())
	}
}

void AMyHUD::BeginPlay()
{
	Super::BeginPlay();

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
	// if(MyCharacter->Children.Num() < 1)
	// {
	// 	UE_LOG(LogMyGame, Warning, TEXT("%s: BeginPlay: no children"), *GetFullName())
	// 	bHasProblems = true;
	// }
	// at BeginPlay, the orbit hasn't replicated to the client yet,
	// thus 'Children' is empty
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
		bSuccessfulInitialization = true;
		//if(bOrbitNetInitDone || GetLocalRole() == ROLE_Authority)
		if(bOrbitNetInitDone || MyCharacter->GetLocalRole() == ROLE_Authority)
		{
			SetActorTickEnabled(true);
			UE_LOG(LogMyGame, Display, TEXT("%s: BeginPlay: OrbitNetInit done, tick enabled"), *GetFullName())
		}
		else
		{
			UE_LOG(LogMyGame, Error, TEXT("%s: waiting for OrbitNetInit"), *GetFullName())
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
	AOrbit* Orbit = Cast<AOrbit>(MyCharacter->Children[0]);
	const float Velocity = Orbit->GetScalarVelocity();

	WidgetHUD->TextVelocitySI->SetText(FText::AsNumber(Velocity * Physics.ScaleFactor, &FormattingOptions));
	WidgetHUD->TextVelocityVCircle->SetText(
		FText::AsNumber(Velocity / Orbit->GetCircleVelocity(Physics), &FormattingOptions));

	const float Angle =
		FQuat::FindBetween
			( FVector(1, 0, 0)
			, Orbit->GetVecVelocity()).GetNormalized().GetTwistAngle(FVector(0, 0, 1)
			);
	WidgetHUD->TextVelocityDirection->SetRenderTransformAngle(Angle * 180. / PI);

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
		WidgetHUD->F1MarkerHide();
		WidgetHUD->CanvasCenterOfMass->SetVisibility(ESlateVisibility::Visible);
		
		float OverlayX, OverlayY;

		const auto Slot = UWidgetLayoutLibrary::SlotAsCanvasSlot(WidgetHUD->CanvasCenterOfMass);

		if(abs(Pos.Y) > abs(Pos.X) / (1. - 2 * XArc(-0.5)))
		{
			// vertical case
			OverlayX = Pos.X * 0.5 / abs(Pos.Y);
			if(Pos.Y < 0)
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
			OverlayY = Pos.Y * (0.5 - XArc(-0.5)) / abs(Pos.X);
			if(Pos.X < 0)
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
		
		UWidgetLayoutLibrary::SlotAsCanvasSlot
			(WidgetHUD->CanvasCenterOfMass)->SetPosition
				( CenterToScreenScaled(this, FVector2D(OverlayX, OverlayY) )
			);
		WidgetHUD->ImgPointer->SetRenderTransformAngle(atan2(Pos.Y, Pos.X) * 180. / PI + 135);
	}
	// on-screen
	else
	{
		WidgetHUD->CanvasCenterOfMass->SetVisibility(ESlateVisibility::Collapsed);
		WidgetHUD->SetF1Marker(CenterToScreenScaled(this, Pos));
	}
}
