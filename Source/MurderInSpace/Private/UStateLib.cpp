// Fill out your copyright notice in the Description page of Project Settings.


#include "UStateLib.h"

#include "MyGameInstance.h"
#include "MyGameState.h"
#include "MyPlayerState.h"

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
	const auto GS = Object->GetWorld()->GetGameState<AMyGameState>();
	if(!GS)
	{
		UE_LOG(LogGameState, Error, TEXT("%s: GameState null"), *Object->GetFullName())
		return DefaultPhysics;
	}
	return GS->Physics;
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
	const TObjectPtr<AMyGameState> GS = World->GetGameState<AMyGameState>();
	const TObjectPtr<UMyGameInstance> GI = World->GetGameInstance<UMyGameInstance>();
		
	if(!GS)
	{
		UE_LOG(LogGameState, Error, TEXT("%s: GameState null"), *Object->GetFullName())
		return FRnd();
	}
	if(!GI)
	{
		UE_LOG(LogGameState, Error, TEXT("%s: GameInstance null"), *Object->GetFullName())
		return FRnd();
	}
	return FRnd
		{ GS->RndGen
		, GS->Poisson
		, GI->Random
		};
}

void UStateLib::WithPlayerUIUnsafe(const UObject* Object, const std::function<FPlayerUI(FPlayerUI)>& Func)
{
	const auto PS = Object->GetWorld()->GetFirstPlayerController()->GetPlayerState<AMyPlayerState>();
	if(!PS)
	{
		UE_LOG(LogPlayerController, Error, TEXT("%s: PlayerState null"), *Object->GetFullName())
		return;
	}
	PS->PlayerUI = Func(PS->PlayerUI);
}
