// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MyPlayerController.h"

#include <algorithm>
#include "Actors/CharacterInSpace.h"
#include "HUD/MyHUD.h"
#include "Modes/MyLocalPlayer.h"
#include "Modes/MyPlayerState.h"

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	const FInputModeGameOnly InputModeGameOnly;
	SetInputMode(InputModeGameOnly);
	
	InputComponent->BindAxis("MouseWheel", this, &AMyPlayerController::Zoom);
	InputComponent->BindAction("Accelerate", IE_Pressed, this, &AMyPlayerController::HandleBeginAccelerate);
	InputComponent->BindAction("Accelerate", IE_Released, this, &AMyPlayerController::HandleEndAccelerate);
	InputComponent->BindAction("ShowMyTrajectory", IE_Pressed, this, &AMyPlayerController::HandleBeginShowMyTrajectory);
	InputComponent->BindAction("ShowMyTrajectory", IE_Released, this, &AMyPlayerController::HandleEndShowMyTrajectory);
	InputComponent->BindAction("ShowAllTrajectories", IE_Pressed, this, &AMyPlayerController::HandleBeginShowAllTrajectories);
	InputComponent->BindAction("ShowAllTrajectories", IE_Released, this, &AMyPlayerController::HandleEndShowAllTrajectories);
	InputComponent->BindAction("Escape", IE_Pressed, this, &AMyPlayerController::HandleEscape);
}

void AMyPlayerController::Zoom(float Delta)
{
	if(!Cast<UMyLocalPlayer>(GetLocalPlayer())->GetIsInMainMenu())
	{
		if(abs(Delta) > 0)
		{
			CameraPosition = std::clamp<int8>(CameraPosition - Delta, 0, MaxCameraPosition);
			GetPawn<ACharacterInSpace>()->UpdateSpringArm(CameraPosition);
			GetPawn<ACharacterInSpace>()->SetVisibility(CameraPosition != 0);

			// TODO: at `CameraPosition = 0` lookAt mouse doesn't work anymore
		}		
	}
}

void AMyPlayerController::HandleEndAccelerate()
{
	if(!Cast<UMyLocalPlayer>(GetLocalPlayer())->GetIsInMainMenu())
	{
		const TObjectPtr<ACharacterInSpace> MyCharacter = GetPawn<ACharacterInSpace>();
		MyCharacter->bIsAccelerating = false;

		const FPlayerUI PlayerUI = UStateLib::GetPlayerUIUnsafe(this, FLocalPlayerContext(this));
		const TObjectPtr<UOrbitComponent> Orbit = MyCharacter->GetOrbitComponent();
		Orbit->bIsVisibleAccelerating = false;
		Orbit->UpdateVisibility(PlayerUI);
		MyCharacter->DestroyTempSplineMesh();
	}
}

void AMyPlayerController::HandleBeginShowMyTrajectory()
{
	if(!Cast<UMyLocalPlayer>(GetLocalPlayer())->GetIsInMainMenu())
	{
		const auto Orbit = GetPawn<ACharacterInSpace>()->GetOrbitComponent();
		const FPlayerUI PlayerUI = UStateLib::GetPlayerUIUnsafe(this, FLocalPlayerContext(this));
		// abusing this variable
		Orbit->bIsVisibleVarious = true;
		Orbit->UpdateVisibility(PlayerUI);
	}
}

void AMyPlayerController::HandleEndShowMyTrajectory()
{
	if(!Cast<UMyLocalPlayer>(GetLocalPlayer())->GetIsInMainMenu())
	{
		const auto Orbit = GetPawn<ACharacterInSpace>()->GetOrbitComponent();
		const FPlayerUI PlayerUI = UStateLib::GetPlayerUIUnsafe(this, FLocalPlayerContext(this));
		// abusing this variable
		Orbit->bIsVisibleVarious = false;
		Orbit->UpdateVisibility(PlayerUI);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AMyPlayerController::HandleBeginShowAllTrajectories()
{
	if(!Cast<UMyLocalPlayer>(GetLocalPlayer())->GetIsInMainMenu())
	{
		SetShowAllTrajectories(true);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AMyPlayerController::HandleEndShowAllTrajectories()
{
	if(!Cast<UMyLocalPlayer>(GetLocalPlayer())->GetIsInMainMenu())
	{
		SetShowAllTrajectories(false);
	}
}

void AMyPlayerController::HandleEscape()
{
	UMyLocalPlayer* LocalPlayer = Cast<UMyLocalPlayer>(GetLocalPlayer());
	if(!LocalPlayer->GetIsInMainMenu())
	{
		if(LocalPlayer->ShowInGameMenu)
		{
			GetHUD<AMyHUD>()->InGameMenuHide();
			CurrentMouseCursor = EMouseCursor::Crosshairs;
			LocalPlayer->ShowInGameMenu = false;
		}
		else
		{
			GetHUD<AMyHUD>()->InGameMenuShow();
			CurrentMouseCursor = EMouseCursor::Default;
			LocalPlayer->ShowInGameMenu = true;
		}
	}
	// TODO: handle escape and other keys in HUD
	// case EInstanceState::WaitingForStart:
	// 	GI->GotoInMenuMain(this);
}

void AMyPlayerController::SetShowAllTrajectories(bool bInShow) const
{
	if(!Cast<UMyLocalPlayer>(GetLocalPlayer())->GetIsInMainMenu())
	{
		UStateLib::WithPlayerUIUnsafe(this, FLocalPlayerContext(this), [this, bInShow] (FPlayerUI PlayerUI)
		{
			PlayerUI.bShowAllTrajectories = bInShow;
			for(TObjectIterator<UOrbitComponent> Iter; Iter; ++Iter)
			{
				(*Iter)->UpdateVisibility(PlayerUI);
			}
		});
	}
}

void AMyPlayerController::HandleBeginAccelerate()
{
	if(!Cast<UMyLocalPlayer>(GetLocalPlayer())->GetIsInMainMenu())
	{
		const TObjectPtr<ACharacterInSpace> MyCharacter = GetPawn<ACharacterInSpace>();
		MyCharacter->bIsAccelerating = true;
		
		const FPlayerUI PlayerUI = UStateLib::GetPlayerUIUnsafe(this, FLocalPlayerContext(this));
		const TObjectPtr<UOrbitComponent> Orbit = MyCharacter->GetOrbitComponent();
		Orbit->SpawnSplineMesh(MyCharacter->GetTempSplineMeshColor(), MyCharacter->GetTempSplineMeshParent(), PlayerUI);
		Orbit->bIsVisibleAccelerating = true;
		Orbit->UpdateVisibility(PlayerUI);
	}
}

// reacting to mouse move
void AMyPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(!Cast<UMyLocalPlayer>(GetLocalPlayer())->GetIsInMainMenu())
	{
		ACharacterInSpace* MyCharacter = GetPawn<ACharacterInSpace>();
		if(!MyCharacter)
		{
			UE_LOG(LogPlayerController, Error, TEXT("MyPlayerController: Tick: no character, no pawn"))
			return;
		}
		FVector Position, Direction;
		DeprojectMousePositionToWorld(Position, Direction);
		if(abs(Direction.Z) > 1e-8)
		{
			// TODO: only works for Position.Z == 0
			const auto X = Position.X - Direction.X * Position.Z / Direction.Z;
			const auto Y = Position.Y - Direction.Y * Position.Z / Direction.Z;
			const auto Z = MyCharacter->GetActorLocation().Z;
			// TODO: physical rotation/animation instead
			MyCharacter->LookAt(FVector(X, Y, Z));
		}
	}
}

void AMyPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	ACharacterInSpace* MyCharacter = Cast<ACharacterInSpace>(InPawn);
	if(!MyCharacter)
	{
		UE_LOG(LogPlayerController, Error, TEXT("AMyPlayerController: GetPawn: No Character."))
	}
	else
	{
		MyCharacter->UpdateSpringArm(CameraPosition);
	}
	FInputModeGameAndUI InputModeGameAndUI;
	SetInputMode(InputModeGameAndUI);
}
