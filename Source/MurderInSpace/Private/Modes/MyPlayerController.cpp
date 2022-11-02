// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MyPlayerController.h"

#include <algorithm>
#include "Actors/CharacterInSpace.h"
#include "HUD/MyHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Modes/MyLocalPlayer.h"
#include "Modes/MyPlayerState.h"
#include "Modes/MyGISubsystem.h"

AMyPlayerController::AMyPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// interface actions
	InputComponent->BindAxis("MouseWheel", this, &AMyPlayerController::Zoom);
	
	InputComponent->BindAction("Escape", IE_Pressed, this, &AMyPlayerController::HandleEscape);
	InputComponent->BindAction("ShowMyTrajectory", IE_Pressed, this, &AMyPlayerController::HandleBeginShowMyTrajectory);
	InputComponent->BindAction("ShowMyTrajectory", IE_Released, this, &AMyPlayerController::HandleEndShowMyTrajectory);
	InputComponent->BindAction("ShowAllTrajectories", IE_Pressed, this, &AMyPlayerController::HandleBeginShowAllTrajectories);
	InputComponent->BindAction("ShowAllTrajectories", IE_Released, this, &AMyPlayerController::HandleEndShowAllTrajectories);
	
	// gameplay actions
	InputComponent->BindAction("Accelerate", IE_Pressed, this, &AMyPlayerController::HandleBeginAccelerate);
	InputComponent->BindAction("Accelerate", IE_Released, this, &AMyPlayerController::HandleEndAccelerate);
}

void AMyPlayerController::Zoom(float Delta)
{
	if(!Cast<UMyLocalPlayer>(Player)->GetIsInMainMenu())
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
	if(!Cast<UMyLocalPlayer>(Player)->GetIsInMainMenu())
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
	if(!Cast<UMyLocalPlayer>(Player)->GetIsInMainMenu())
	{
		const auto Orbit = GetPawn<ACharacterInSpace>()->GetOrbitComponent();
		const FPlayerUI PlayerUI = UStateLib::GetPlayerUIUnsafe(this, FLocalPlayerContext(this));
		Orbit->bIsVisibleVarious = true;
		Orbit->UpdateVisibility(PlayerUI);
	}
}

void AMyPlayerController::HandleEndShowMyTrajectory()
{
	if(!Cast<UMyLocalPlayer>(Player)->GetIsInMainMenu())
	{
		const auto Orbit = GetPawn<ACharacterInSpace>()->GetOrbitComponent();
		const FPlayerUI PlayerUI = UStateLib::GetPlayerUIUnsafe(this, FLocalPlayerContext(this));
		Orbit->bIsVisibleVarious = false;
		Orbit->UpdateVisibility(PlayerUI);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AMyPlayerController::HandleBeginShowAllTrajectories()
{
	if(!Cast<UMyLocalPlayer>(Player)->GetIsInMainMenu())
	{
		SetShowAllTrajectories(true);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AMyPlayerController::HandleEndShowAllTrajectories()
{
	if(!Cast<UMyLocalPlayer>(Player)->GetIsInMainMenu())
	{
		SetShowAllTrajectories(false);
	}
}

void AMyPlayerController::HandleEscape()
{
	UMyLocalPlayer* LocalPlayer = Cast<UMyLocalPlayer>(Player);
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
	else
	{
		GetGameInstance<UMyGameInstance>()->MulticastRPC_LeaveSession();
	}
}

void AMyPlayerController::ClientRPC_LeaveSession_Implementation()
{
	GetGameInstance()->GetSubsystem<UMyGISubsystem>()->LeaveSession();
}

void AMyPlayerController::SetShowAllTrajectories(bool bInShow) const
{
	UStateLib::WithPlayerUIUnsafe(this, FLocalPlayerContext(this), [this, bInShow] (FPlayerUI PlayerUI)
	{
		PlayerUI.bShowAllTrajectories = bInShow;
		for(TObjectIterator<UOrbitComponent> Iter; Iter; ++Iter)
		{
			UOrbitComponent* Orbit = *Iter;
			// For PIE multiplayer-testing only: with `Run under one process` checked, the object iterator iterates
			// through all worlds, across host and client
			if(Orbit->GetWorld() == GetWorld())
			{
				Orbit->UpdateVisibility(PlayerUI);
			}
		}
	});
}

void AMyPlayerController::HandleBeginAccelerate()
{
	if(!Cast<UMyLocalPlayer>(Player)->GetIsInMainMenu())
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

	UMyLocalPlayer* MyPlayer = Cast<UMyLocalPlayer>(Player);
	if(!MyPlayer->GetIsInMainMenu())
	{
		ACharacterInSpace* MyCharacter = GetPawn<ACharacterInSpace>();
		FVector Position, Direction;
		DeprojectMousePositionToWorld(Position, Direction);
		if(abs(Direction.Z) > 1e-8)
		{
			// TODO: only works for Position.Z == 0
			const float X = Position.X - Direction.X * Position.Z / Direction.Z;
			const float Y = Position.Y - Direction.Y * Position.Z / Direction.Z;
			const float Z = MyCharacter->GetActorLocation().Z;
			// TODO: physical rotation/animation instead
			MyCharacter->LookAt(FVector(X, Y, Z));
		}
	}
}

void AMyPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);
	
	SetActorTickEnabled(true);
	Cast<ACharacterInSpace>(P)->UpdateSpringArm(CameraPosition);
	
	const FString LevelName = UGameplayStatics::GetCurrentLevelName(GetWorld());
	ECurrentLevel CurrentLevel;
	if(LevelName == FString(TEXT("MainMenu")))
	{
		CurrentLevel = ECurrentLevel::MainMenu;
	}
	else if(LevelName == FString(TEXT("Spacefootball")))
	{
		CurrentLevel = ECurrentLevel::SpaceFootball;
	}
	else
	{
		UE_LOG(LogNet, Error, TEXT("%s: Couldn't determine current level: %s"), *GetFullName(), *LevelName)
		CurrentLevel = ECurrentLevel::MainMenu;
	}
	Cast<UMyLocalPlayer>(GetLocalPlayer())->CurrentLevel = CurrentLevel;
}
