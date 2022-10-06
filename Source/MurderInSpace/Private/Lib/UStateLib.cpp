// Fill out your copyright notice in the Description page of Project Settings.


#include "Lib/UStateLib.h"

#include "Modes/MyGameInstance.h"
#include "Modes/MyGameState.h"
#include "Modes/MyPlayerState.h"

float UStateLib::GetInitialAngularVelocity(FRnd Rnd)
{
	return Rnd.Poisson(Rnd.RndGen) / 1.e3;
}

FPhysics UStateLib::GetPhysics(AMyGameState* GS)
{
	return GS->Physics;
}

FPhysics UStateLib::GetPhysicsUnsafe(const UObject* Object)
{
	const auto World = Object->GetWorld();
	const TObjectPtr<UMyGameInstance> GI = World->GetGameInstance<UMyGameInstance>();
	if(!GI)
	{
		UE_LOG
			( LogGameState
			, Error
			, TEXT("UStateLib::GetPhysicsUnsafe: %s: Game instance null")
			, *Object->GetFullName()
			)
		return DefaultPhysics;
	}
	switch(GI->InstanceState)
	{
	case EInstanceState::InMenuMain:
	case EInstanceState::InMenuServers:
	case EInstanceState::InGame:
	case EInstanceState::InGameMenu:
		const TObjectPtr<AMyGameState> GS = World->GetGameState<AMyGameState>();
		if(!GS)
		{
			UE_LOG
				( LogGameState
				, Error
				, TEXT("UStateLib::GetPhysicsUnsafe: %s: GameState null")
				, *Object->GetFullName()
				)
			return DefaultPhysics;
		}
		return GS->Physics;
	default:
		UE_LOG
			( LogGameState
			, Error
			, TEXT("UStateLib::GetPhysicsUnsafe: %s: Instance state: %s, expected InMenu* or InGame*")
			, *Object->GetFullName()
			, *UEnum::GetValueAsString(GI->InstanceState)
			)
		return DefaultPhysics;
	}
}

FPhysics UStateLib::GetPhysicsEditorDefault()
{
	return DefaultPhysics;
}

FPlayerUI UStateLib::GetPlayerUI(AMyPlayerState* PS)
{
	return PS->PlayerUI;
}

FPlayerUI UStateLib::GetPlayerUIUnsafe(UObject* Object)
{
	const auto PS = Object->GetWorld()->GetFirstPlayerController()->GetPlayerState<AMyPlayerState>();
	if(!PS)
	{
		UE_LOG(LogPlayerController, Error, TEXT("%s: PlayerState null"), *Object->GetFullName())
		return DefaultPlayerUI;
	}
	return PS->PlayerUI;
}

FPlayerUI UStateLib::GetPlayerUIEditorDefault()
{
	return DefaultPlayerUI;
}

FRnd UStateLib::GetRnd(AMyGameState* GS, UMyGameInstance* GI)
{
	return FRnd
		{ GS->RndGen
        , GS->Poisson
        , GI->Random
        };
}

FRnd UStateLib::GetRndUnsafe(UObject* Object)
{
	const auto World = Object->GetWorld();
	const TObjectPtr<UMyGameInstance> GI = World->GetGameInstance<UMyGameInstance>();
	if(!GI)
	{
		UE_LOG
			( LogGameState
			, Error
			, TEXT("UStateLib::GetRndUnsafe: %s: GameInstance null")
			, *Object->GetFullName()
			)
		return FRnd();
	}
	switch(GI->InstanceState)
	{
	case EInstanceState::InMenuMain:
	case EInstanceState::InMenuServers:
	case EInstanceState::InGame:
	case EInstanceState::InGameMenu:
		const TObjectPtr<AMyGameState> GS = World->GetGameState<AMyGameState>();
			
		if(!GS)
		{
			UE_LOG
				( LogGameState
				, Error
				, TEXT("UStateLib::GetRndUnsafe: %s: GameState null")
				, *Object->GetFullName()
				)
			return FRnd();
		}
		return FRnd
			{ GS->RndGen
			, GS->Poisson
			, GI->Random
			};
	default:
		UE_LOG
			( LogGameState
			, Error
			, TEXT("UStateLib::GetRndUnsafe: %s: Instance state: %s, expected InMenu* or InGame*")
			, *Object->GetFullName()
			, *UEnum::GetValueAsString(GI->InstanceState)
			)
		return FRnd();
	}
}

void UStateLib::ModifyPlayerUIUnsafe(const UObject* Object, const std::function<FPlayerUI(FPlayerUI)>& Func)
{
	const auto PS = Object->GetWorld()->GetFirstPlayerController()->GetPlayerState<AMyPlayerState>();
	if(!PS)
	{
		UE_LOG(LogPlayerController, Error, TEXT("%s: PlayerState null"), *Object->GetFullName())
		return;
	}
	PS->PlayerUI = Func(PS->PlayerUI);
}

void UStateLib::SetInstanceState(UMyGameInstance* GI, EInstanceState InNewState)
{
	GI->InstanceState = InNewState;
}
