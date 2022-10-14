// Fill out your copyright notice in the Description page of Project Settings.


#include "Lib/UStateLib.h"

#include "Modes/MyGameInstance.h"
#include "Modes/MyGameState.h"
#include "Modes/MyPlayerState.h"

float UStateLib::GetInitialAngularVelocity(FRnd Rnd)
{
	return Rnd.Poisson(Rnd.RndGen) / 1.e3;
}

FPhysics UStateLib::GetPhysics(const AMyGameState* GS)
{
	return GS->Physics;
}

FPhysics UStateLib::GetPhysicsUnsafe(const UObject* Object)
{
	const UWorld* World = Object->GetWorld();
	UMyGameInstance* GI = World->GetGameInstance<UMyGameInstance>();
	if(!GI)
	{
		const UGameInstance* GIGeneric = World->GetGameInstance();
		UE_LOG
			( LogGameState
			, Error
			, TEXT("UStateLib::GetPhysicsUnsafe: %s: UMyGameInstance null; generic game instance: %s")
			, *Object->GetFullName()
			, GIGeneric ? *GIGeneric->GetFullName() : TEXT("null")
			)
		return DefaultPhysics;
	}
	
	const AMyGameState* GS = World->GetGameState<AMyGameState>();
	if(!GS)
	{
		const AGameState* GSGeneric = World->GetGameState<AGameState>();
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
}

FPhysics UStateLib::GetPhysicsEditorDefault()
{
	return DefaultPhysics;
}

FPlayerUI UStateLib::GetPlayerUI(const AMyPlayerState* PS)
{
	return PS->PlayerUI;
}

FPlayerUI UStateLib::GetPlayerUIUnsafe(const UObject* Object, const FLocalPlayerContext& LPC)
{
	const AMyPlayerState* PS = LPC.GetPlayerState<AMyPlayerState>();
	if(!PS)
	{
		const APlayerState* PSGeneric = LPC.GetPlayerState<APlayerState>();
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

FRnd UStateLib::GetRnd(const AMyGameState* GS, const UMyGameInstance* GI)
{
	return FRnd
		{ GS->RndGen
        , GS->Poisson
        , GI->Random
        };
}

FRnd UStateLib::GetRndUnsafe(const UObject* Object)
{
	const UWorld* World = Object->GetWorld();
	const UMyGameInstance* GI = World->GetGameInstance<UMyGameInstance>();
	if(!GI)
	{
		const UGameInstance* GIGeneric = World->GetGameInstance();
		UE_LOG
			( LogGameState
			, Error
			, TEXT("UStateLib::GetRndUnsafe: %s: UMyGameInstance null; UGameInstance: %s")
			, *Object->GetFullName()
			, GIGeneric ? *GIGeneric->GetFullName() : TEXT("null")
			)
		return FRnd();
	}
	
	const AMyGameState* GS = World->GetGameState<AMyGameState>();
	if(!GS)
	{
		const AGameState* GSGeneric = World->GetGameState<AGameState>();
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
}

void UStateLib::WithPlayerUIUnsafe(const UObject* Object, const FLocalPlayerContext& LPC, const TFunctionRef<void(FPlayerUI&)> Func)
{
	AMyPlayerState* PS = LPC.GetPlayerState<AMyPlayerState>();
	if(!PS)
	{
		const APlayerState* PSGeneric = LPC.GetPlayerState<APlayerState>();
		UE_LOG
			( LogPlayerController
			, Error
			, TEXT("UStateLib::ModifyPlayerUIUnsafe: %s: AMyPlayerState null; APlayerState: %s")
			, *Object->GetFullName()
			, PSGeneric ? *PSGeneric->GetFullName() : TEXT("null")
			)
		return;
	}
	Func(PS->PlayerUI);
}

// void UStateLib::SetInstanceState(UMyGameInstance* GI, int32 PlayerNum, const EInstanceState InNewState)
// {
// 	GI->InstanceStateArray[PlayerNum] = InNewState;
// }
