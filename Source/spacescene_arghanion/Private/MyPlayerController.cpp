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

void AMyPlayerController::MouseMove(FVector VecDelta)
{
	FVector Position, Direction;
	DeprojectMousePositionToWorld(Position, Direction);
	if(abs(Direction.Z) > 1e-8)
	{
		const auto X = Position.X - Direction.X * Position.Z / Direction.Z;
		const auto Y = Position.Y - Direction.Y * Position.Z / Direction.Z;
		GetPawn<ACharacterInSpace>()->LookAt(FVector(X, Y, 0));
		//UE_LOG(LogTemp, Warning, TEXT("(%f, %f, %f) - (%f, %f)"), Delta.X, Delta.Y, Delta.Z, X, Y);
	}
}

void AMyPlayerController::MouseWheel(float Delta)
{
	if(Delta > 0)
	{
		CameraPosition = std::clamp<uint8>(CameraPosition + Delta, 0, MaxCameraPosition);
		UE_LOG(LogTemp, Warning, TEXT("MouseWheel: %f"), Delta);
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
	UE_LOG(LogTemp, Warning, TEXT("HandeMouseMove, (%f, %f, %f) - (%f, %f, %f)"), Position.X, Position.Y, Position.Z, Direction.X, Direction.Y, Direction.Z);
	if(abs(Direction.Z) > 1e-8)
	{
		const auto X = Position.X + Direction.X * Position.Z / Direction.Z;
		const auto Y = Position.Y + Direction.Y * Position.Z / Direction.Z;
		GetPawn<ACharacterInSpace>()->LookAt(FVector(X, Y, 0));
		UE_LOG(LogTemp, Warning, TEXT("%f - (%f, %f)"), Delta, X, Y);
	}
}
