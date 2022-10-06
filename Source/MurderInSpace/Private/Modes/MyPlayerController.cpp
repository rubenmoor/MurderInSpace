// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MyPlayerController.h"

#include <algorithm>

#include "Actors/CharacterInSpace.h"
#include "Modes/MyPlayerState.h"

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
	InputComponent->BindAction("Escape", IE_Pressed, this, &AMyPlayerController::HandleEscape);
}

void AMyPlayerController::Zoom(float Delta)
{
	if(GetGameInstance<UMyGameInstance>()->GetInstanceState() == EInstanceState::InGame)
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
	if(GetGameInstance<UMyGameInstance>()->GetInstanceState() == EInstanceState::InGame)
	{
		const TObjectPtr<ACharacterInSpace> MyCharacter = GetPawn<ACharacterInSpace>();
		MyCharacter->bIsAccelerating = false;

		const FPlayerUI PlayerUI = UStateLib::GetPlayerUIUnsafe(this);
		const TObjectPtr<UOrbitComponent> Orbit = MyCharacter->GetOrbitComponent();
		Orbit->bIsVisibleAccelerating = false;
		Orbit->UpdateVisibility(PlayerUI);
		MyCharacter->DestroyTempSplineMesh();
	}
}

void AMyPlayerController::HandleBeginShowMyTrajectory()
{
	if(GetGameInstance<UMyGameInstance>()->GetInstanceState() == EInstanceState::InGame)
	{
		const auto Orbit = GetPawn<ACharacterInSpace>()->GetOrbitComponent();
		const FPlayerUI PlayerUI = UStateLib::GetPlayerUIUnsafe(this);
		// abusing this variable
		Orbit->bIsVisibleVarious = true;
		Orbit->UpdateVisibility(PlayerUI);
	}
}

void AMyPlayerController::HandleEndShowMyTrajectory()
{
	if(GetGameInstance<UMyGameInstance>()->GetInstanceState() == EInstanceState::InGame)
	{
		const auto Orbit = GetPawn<ACharacterInSpace>()->GetOrbitComponent();
		const FPlayerUI PlayerUI = UStateLib::GetPlayerUIUnsafe(this);
		// abusing this variable
		Orbit->bIsVisibleVarious = false;
		Orbit->UpdateVisibility(PlayerUI);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AMyPlayerController::HandleBeginShowAllTrajectories()
{
	if(GetGameInstance<UMyGameInstance>()->GetInstanceState() == EInstanceState::InGame)
	{
		SetShowAllTrajectories(true);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AMyPlayerController::HandleEndShowAllTrajectories()
{
	if(GetGameInstance<UMyGameInstance>()->GetInstanceState() == EInstanceState::InGame)
	{
		SetShowAllTrajectories(false);
	}
}

void AMyPlayerController::HandleEscape()
{
	switch(const TObjectPtr<UMyGameInstance> GI = GetGameInstance<UMyGameInstance>(); GI->GetInstanceState())
	{
	case EInstanceState::InGame:
		GI->GotoInGameMenu();
		CurrentMouseCursor = EMouseCursor::Default;
		break;
	case EInstanceState::InGameMenu:
		GI->GotoInGame();
		CurrentMouseCursor = EMouseCursor::Crosshairs;
		break;
	default:
		// nothing to do here
		break;
	}
}

void AMyPlayerController::SetShowAllTrajectories(bool bInShow) const
{
	if(GetGameInstance<UMyGameInstance>()->GetInstanceState() == EInstanceState::InGame)
	{
		UStateLib::ModifyPlayerUIUnsafe(this, [this, bInShow] (FPlayerUI PlayerUI)
			{
				PlayerUI.bShowAllTrajectories = bInShow;
				for(TObjectIterator<UOrbitComponent> Iter; Iter; ++Iter)
				{
					(*Iter)->UpdateVisibility(PlayerUI);
				}
				return PlayerUI;
			});
	}
}

void AMyPlayerController::HandleBeginAccelerate()
{
	if(GetGameInstance<UMyGameInstance>()->GetInstanceState() == EInstanceState::InGame)
	{
		const TObjectPtr<ACharacterInSpace> MyCharacter = GetPawn<ACharacterInSpace>();
		MyCharacter->bIsAccelerating = true;
		
		const FPlayerUI PlayerUI = UStateLib::GetPlayerUIUnsafe(this);
		const TObjectPtr<UOrbitComponent> Orbit = MyCharacter->GetOrbitComponent();
		Orbit->SpawnSplineMesh(MyCharacter->GetTempSplineMeshColor(), MyCharacter->GetTempSplineMeshParent(), PlayerUI);
		Orbit->bIsVisibleAccelerating = true;
		Orbit->UpdateVisibility(PlayerUI);
	}
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

	if(GetGameInstance<UMyGameInstance>()->GetInstanceState() == EInstanceState::InGame)
	{
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
}
