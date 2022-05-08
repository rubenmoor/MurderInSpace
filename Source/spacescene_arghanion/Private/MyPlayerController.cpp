// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"

#include <algorithm>

#include "CharacterInSpace.h"
#include "MeshAttributes.h"

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAxis("MouseWheel", this, &AMyPlayerController::Zoom);
	InputComponent->BindAction("Accelerate", IE_Pressed, this, &AMyPlayerController::AccelerateBegin);
	InputComponent->BindAction("Accelerate", IE_Released, this, &AMyPlayerController::AccelerateEnd);
}

AMyPlayerController::AMyPlayerController()
{
	CameraPosition = 2;
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

void AMyPlayerController::AccelerateEnd()
{
	GetPawn<APawnInSpace>()->bIsAccelerating = false;
}

void AMyPlayerController::AccelerateBegin()
{
	GetPawn<APawnInSpace>()->bIsAccelerating = true;
}

void AMyPlayerController::BeginPlay()
{
	GetPawn<ACharacterInSpace>()->UpdateSpringArm(CameraPosition);
	Super::BeginPlay();
}

void AMyPlayerController::Tick(float DeltaSeconds)
{
	FVector Position, Direction;
	DeprojectMousePositionToWorld(Position, Direction);
	if(abs(Direction.Z) > 1e-8)
	{
		const auto X = Position.X - Direction.X * Position.Z / Direction.Z;
		const auto Y = Position.Y - Direction.Y * Position.Z / Direction.Z;
		GetPawn<ACharacterInSpace>()->LookAt(FVector(X, Y, 0));
	}
	
	Super::Tick(DeltaSeconds);
}
