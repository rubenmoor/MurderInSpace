// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"

#include <algorithm>

#include "CharacterInSpace.h"
#include "MeshAttributes.h"

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	// doesn't work
	// Error: Ensure condition failed: AxisKey.IsAxis2D() || AxisKey.IsAxis3D()
	// always fires, VecDelta always zero
	InputComponent->BindVectorAxis("MouseMove", this, &AMyPlayerController::MouseMove);

	InputComponent->BindAxis("MouseWheel", this, &AMyPlayerController::MouseWheel);
	
	//InputComponent->BindAxis("MouseMoveX", this, &AMyPlayerController::MouseMoveX);
	//InputComponent->BindAxis("MouseMoveY", this, &AMyPlayerController::MouseMoveY);
}

AMyPlayerController::AMyPlayerController()
{
	CameraPosition = 2;
}

void AMyPlayerController::MouseMove(FVector VecDelta)
{
	FVector Position, Direction;
	DeprojectMousePositionToWorld(Position, Direction);
	if(abs(Direction.Z) > 1e-8)
	{
		const auto X = Position.X - Direction.X * Position.Z / Direction.Z;
		const auto Y = Position.Y - Direction.Y * Position.Z / Direction.Z;
		GetPawn<ACharacterInSpace>()->LookAt(FVector(X, Y, 0));
	}
}

void AMyPlayerController::MouseWheel(float Delta)
{
	if(abs(Delta) > 0)
	{
		CameraPosition = std::clamp<int8>(CameraPosition - Delta, 0, MaxCameraPosition);
		GetPawn<ACharacterInSpace>()->UpdateSpringArm(CameraPosition);
	}		
}

void AMyPlayerController::MouseMoveX(float Delta)
{
	HandleMouseMove(Delta);
}

void AMyPlayerController::MouseMoveY(float Delta)
{
	HandleMouseMove(Delta);
}

void AMyPlayerController::HandleMouseMove(float Delta) const
{
	FVector Position, Direction;
	DeprojectMousePositionToWorld(Position, Direction);
	if(abs(Direction.Z) > 1e-8)
	{
		const auto X = Position.X + Direction.X * Position.Z / Direction.Z;
		const auto Y = Position.Y + Direction.Y * Position.Z / Direction.Z;
		GetPawn<ACharacterInSpace>()->LookAt(FVector(X, Y, 0));
	}
}

void AMyPlayerController::BeginPlay()
{
	GetPawn<ACharacterInSpace>()->UpdateSpringArm(CameraPosition);
	Super::BeginPlay();
}
