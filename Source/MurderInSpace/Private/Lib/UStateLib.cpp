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
	const TObjectPtr<UWorld> World = Object->GetWorld();
	const TObjectPtr<UMyGameInstance> GI = World->GetGameInstance<UMyGameInstance>();
	if(!GI)
	{
		const TObjectPtr<UGameInstance> GIGeneric = World->GetGameInstance();
		UE_LOG
			( LogGameState
			, Error
			, TEXT("UStateLib::GetPhysicsUnsafe: %s: UMyGameInstance null; generic game instance: %s")
			, *Object->GetFullName()
			, GIGeneric ? *GIGeneric->GetFullName() : TEXT("null")
			)
		return DefaultPhysics;
	}
	switch(GI->InstanceState)
	{
	case EInstanceState::InMainMenu:
	case EInstanceState::WaitingForSessionCreate:
	case EInstanceState::InGame:
		const TObjectPtr<AMyGameState> GS = World->GetGameState<AMyGameState>();
		if(!GS)
		{
			const TObjectPtr<AGameState> GSGeneric = World->GetGameState<AGameState>();
			UE_LOG
				( LogGameState
				, Error
				, TEXT("UStateLib::GetPhysicsUnsafe: %s: AMyGameState null; AGameState: %s")
				, *Object->GetFullName()
				, GSGeneric ? *GSGeneric->GetFullName() : TEXT("null")
				)
			return DefaultPhysics;
		}
		return GS->Physics;
	default:
		GI->ErrorWrongState
			( Object
			, UEnum::GetValueAsString(EInstanceState::InMainMenu)
			+ UEnum::GetValueAsString(EInstanceState::WaitingForSessionCreate)
			+ UEnum::GetValueAsString(EInstanceState::InGame)
			);
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
	const TObjectPtr<AMyPlayerState> PS = Object->GetWorld()->GetFirstPlayerController()->GetPlayerState<AMyPlayerState>();
	if(!PS)
	{
		const TObjectPtr<APlayerState> PSGeneric = Object->GetWorld()->GetFirstPlayerController()->GetPlayerState<APlayerState>();
		UE_LOG
			( LogPlayerController
			, Error
			, TEXT("UStateLib::GetPlayerUIUnsafe: %s: AMyPlayerState null; APlayerState: %s")
			, *Object->GetFullName()
			, PSGeneric ? *PSGeneric->GetFullName() : TEXT("null")
			)
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
	const TObjectPtr<UWorld> World = Object->GetWorld();
	const TObjectPtr<UMyGameInstance> GI = World->GetGameInstance<UMyGameInstance>();
	if(!GI)
	{
		const TObjectPtr<UGameInstance> GIGeneric = World->GetGameInstance();
		UE_LOG
			( LogGameState
			, Error
			, TEXT("UStateLib::GetRndUnsafe: %s: UMyGameInstance null; UGameInstance: %s")
			, *Object->GetFullName()
			, GIGeneric ? *GIGeneric->GetFullName() : TEXT("null")
			)
		return FRnd();
	}
	switch(GI->InstanceState)
	{
	case EInstanceState::InMainMenu:
	case EInstanceState::InGame:
		const TObjectPtr<AMyGameState> GS = World->GetGameState<AMyGameState>();
			
		if(!GS)
		{
			const TObjectPtr<AGameState> GSGeneric = World->GetGameState<AGameState>();
			UE_LOG
				( LogGameState
				, Error
				, TEXT("UStateLib::GetRndUnsafe: %s: AMyGameState null; AGameState: %s")
				, *Object->GetFullName()
				, GSGeneric ? *GSGeneric->GetFullName() : TEXT("null")
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

void UStateLib::WithPlayerUIUnsafe(const UObject* Object, const TFunctionRef<FPlayerUI(FPlayerUI)> Func)
{
	const TObjectPtr<AMyPlayerState> PS = Object->GetWorld()->GetFirstPlayerController()->GetPlayerState<AMyPlayerState>();
	if(!PS)
	{
		const TObjectPtr<APlayerState> PSGeneric = Object->GetWorld()->GetFirstPlayerController()->GetPlayerState<APlayerState>();
		UE_LOG
			( LogPlayerController
			, Error
			, TEXT("UStateLib::ModifyPlayerUIUnsafe: %s: AMyPlayerState null; APlayerState: %s")
			, *Object->GetFullName()
			, PSGeneric ? *PSGeneric->GetFullName() : TEXT("null")
			)
		return;
	}
	PS->PlayerUI = Func(PS->PlayerUI);
}

void UStateLib::SetInstanceState(UMyGameInstance* GI, EInstanceState InNewState)
{
	GI->InstanceState = InNewState;
}
