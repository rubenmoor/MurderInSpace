// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"

#include <algorithm>

#include "CharacterInSpace.h"

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAxis("MouseWheel", this, &AMyPlayerController::Zoom);
	InputComponent->BindAction("Accelerate", IE_Pressed, this, &AMyPlayerController::AccelerateBegin);
	InputComponent->BindAction("Accelerate", IE_Released, this, &AMyPlayerController::AccelerateEnd);
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
	TObjectPtr<ACharacterInSpace> MyCharacter = GetPawn<ACharacterInSpace>();
	MyCharacter->bIsAccelerating = false;
	MyCharacter->GetOrbitComponent()->Hide();
}

void AMyPlayerController::AccelerateBegin()
{
	TObjectPtr<ACharacterInSpace> MyCharacter = GetPawn<ACharacterInSpace>();
	MyCharacter->bIsAccelerating = true;
	MyCharacter->GetOrbitComponent()->Show();
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
