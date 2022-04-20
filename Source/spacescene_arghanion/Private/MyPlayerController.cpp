// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"

#include "CharacterInSpace.h"
#include "MeshAttributes.h"

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindVectorAxis("MouseMove", this, &AMyPlayerController::MouseMove);
}

void AMyPlayerController::MouseMove(FVector Delta)
{
	if(GetPawn<ACharacterInSpace>())
	{
		FVector Position, Direction;
		DeprojectMousePositionToWorld(Position, Direction);
		if(Direction.Z > 1e-8)
		{
			const auto X = Position.X + Direction.X * Position.Z / Direction.Z;
			const auto Y = Position.Y + Direction.Y * Position.Z / Direction.Z;
			GetPawn<ACharacterInSpace>()->LookAt(FVector(X, Y, 0));
			UE_LOG(LogTemp, Warning, TEXT("(%f, %f, %f) - (%f, %f)"), Delta.X, Delta.Y, Delta.Z, X, Y);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GetPawn(): null"));
	}
}
