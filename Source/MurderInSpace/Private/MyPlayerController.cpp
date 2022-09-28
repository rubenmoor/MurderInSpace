// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"

#include <algorithm>

#include "CharacterInSpace.h"
#include "MyPlayerState.h"

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAxis("MouseWheel", this, &AMyPlayerController::Zoom);
	InputComponent->BindAction("Accelerate", IE_Pressed, this, &AMyPlayerController::HandleBeginAccelerate);
	InputComponent->BindAction("Accelerate", IE_Released, this, &AMyPlayerController::HandleEndAccelerate);
	InputComponent->BindAction("ShowMyTrajectory", IE_Pressed, this, &AMyPlayerController::HandleBeginShowMyTrajectory);
	InputComponent->BindAction("ShowMyTrajectory", IE_Released, this, &AMyPlayerController::HandleEndShowMyTrajectory);
	InputComponent->BindAction("ShowAllTrajectories", IE_Pressed, this, &AMyPlayerController::HandleBeginShowAllTrajectories);
	InputComponent->BindAction("ShowAllTrajectories", IE_Released, this, &AMyPlayerController::HandleEndShowAllTrajectories);
}

void AMyPlayerController::Zoom(float Delta)
{
	if(abs(Delta) > 0)
	{
		CameraPosition = std::clamp<int8>(CameraPosition - Delta, 0, MaxCameraPosition);
		GetPawn<ACharacterInSpace>()->UpdateSpringArm(CameraPosition);
		GetPawn<ACharacterInSpace>()->SetVisibility(CameraPosition != 0);

		// TODO: at `CameraPosition = 0` lookAt mouse doesn't work anymore
	}		
}

void AMyPlayerController::HandleEndAccelerate()
{
	const TObjectPtr<ACharacterInSpace> MyCharacter = GetPawn<ACharacterInSpace>();
	MyCharacter->bIsAccelerating = false;

	const FPlayerUI PlayerUI = UStateLib::GetPlayerUIUnsafe(this);
	const TObjectPtr<UOrbitComponent> Orbit = MyCharacter->GetOrbitComponent();
	Orbit->bIsVisibleAccelerating = false;
	Orbit->UpdateVisibility(PlayerUI);
	MyCharacter->DestroyTempSplineMesh();
}

void AMyPlayerController::HandleBeginShowMyTrajectory()
{
	const auto Orbit = GetPawn<ACharacterInSpace>()->GetOrbitComponent();
	const FPlayerUI PlayerUI = UStateLib::GetPlayerUIUnsafe(this);
	// abusing this variable
	Orbit->bIsVisibleMouseOver = true;
	Orbit->UpdateVisibility(PlayerUI);
}

void AMyPlayerController::HandleEndShowMyTrajectory()
{
	const auto Orbit = GetPawn<ACharacterInSpace>()->GetOrbitComponent();
	const FPlayerUI PlayerUI = UStateLib::GetPlayerUIUnsafe(this);
	// abusing this variable
	Orbit->bIsVisibleMouseOver = false;
	Orbit->UpdateVisibility(PlayerUI);
}

void AMyPlayerController::HandleBeginShowAllTrajectories()
{
	SetShowAllTrajectories(true);
}

void AMyPlayerController::HandleEndShowAllTrajectories()
{
	SetShowAllTrajectories(false);
}

void AMyPlayerController::SetShowAllTrajectories(bool bInShow) const
{
	UStateLib::WithPlayerUIUnsafe(this, [this, bInShow] (FPlayerUI PlayerUI)
		{
			PlayerUI.bShowAllTrajectories = bInShow;
			for(TObjectIterator<UOrbitComponent> Iter; Iter; ++Iter)
			{
				(*Iter)->UpdateVisibility(PlayerUI);
			}
			return PlayerUI;
		});
}

void AMyPlayerController::HandleBeginAccelerate()
{
	const TObjectPtr<ACharacterInSpace> MyCharacter = GetPawn<ACharacterInSpace>();
	MyCharacter->bIsAccelerating = true;
	
	const FPlayerUI PlayerUI = UStateLib::GetPlayerUIUnsafe(this);
	const TObjectPtr<UOrbitComponent> Orbit = MyCharacter->GetOrbitComponent();
	Orbit->SpawnSplineMesh(MyCharacter->GetTempSplineMeshColor(), MyCharacter->GetTempSplineMeshParent(), PlayerUI);
	Orbit->bIsVisibleAccelerating = true;
	Orbit->UpdateVisibility(PlayerUI);
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	const TObjectPtr<ACharacterInSpace> MyCharacter = GetPawn<ACharacterInSpace>();
	if(!MyCharacter)
	{
		UE_LOG(LogPlayerController, Error, TEXT("AMyPlayerController: GetPawn: No Character."))
	}
	else
	{
		MyCharacter->UpdateSpringArm(CameraPosition);
	}
}

void AMyPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	const TObjectPtr<ACharacterInSpace> MyCharacter = GetPawn<ACharacterInSpace>();
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
