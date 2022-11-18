// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MyState.h"

float UMyState::GetInitialAngularVelocity(FRnd Rnd)
{
	return Rnd.Poisson(Rnd.RndGen) / 1.e3;
}

FPhysics UMyState::GetPhysicsAny(const UObject* Object)
{
	const UWorld* World = Object->GetWorld();
	const UGameInstance* GIGeneric = World->GetGameInstance();
	if(!IsValid(GIGeneric))
	{
		return GetPhysicsEditorDefault();
	}
	UMyGameInstance* GI = World->GetGameInstance<UMyGameInstance>();
	if(!GI)
	{
		UE_LOG
			( LogGameState
			, Error
			, TEXT("UStateLib::GetPhysicsUnsafe: %s: UMyGameInstance null; generic game instance: %s")
			, *Object->GetFullName()
			, GIGeneric ? *GIGeneric->GetFullName() : TEXT("null")
			)
		return GetPhysicsEditorDefault();
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
		return GetPhysicsEditorDefault();
	}
	return GetPhysics(GS);
}

FPlayerUI UMyState::GetPlayerUIAny(const UObject* Object, const FLocalPlayerContext& LPC)
{
	const APlayerState* PSGeneric = LPC.GetPlayerState<APlayerState>();
	if(!IsValid(PSGeneric))
	{
		return GetPlayerUIEditorDefault();
	}
	const AMyPlayerState* PS = LPC.GetPlayerState<AMyPlayerState>();
	if(!PS)
	{
		UE_LOG
			( LogPlayerController
			, Error
			, TEXT("UStateLib::GetPlayerUIUnsafe: %s: AMyPlayerState null; APlayerState: %s")
			, *Object->GetFullName()
			, PSGeneric ? *PSGeneric->GetFullName() : TEXT("null")
			)
		return GetPlayerUIEditorDefault();
	}
	return GetPlayerUI(PS);
}

FInstanceUI UMyState::GetInstanceUIAny(const UObject* Object)
{
	const UWorld* World = Object->GetWorld();
	const UGameInstance* GIGeneric = World->GetGameInstance();
	if(!IsValid(GIGeneric))
	{
		return GetInstanceUIEditorDefault();
	}
	const UMyGameInstance* GI = World->GetGameInstance<UMyGameInstance>();
	if(!GI)
	{
		UE_LOG
		( LogGameState
		, Error
		, TEXT("UStateLib::GetInstanceUIUnsafe: %s: UMyGameInstance null; UGameInstance: %s")
		, *Object->GetFullName()
		, GIGeneric ? *GIGeneric->GetFullName() : TEXT("null")
		)
		return GetInstanceUIEditorDefault();
	}
	return GetInstanceUI(GI);
}

FRnd UMyState::GetRndAny(const UObject* Object)
{
	const UWorld* World = Object->GetWorld();
	const UGameInstance* GIGeneric = World->GetGameInstance();
	if(!IsValid(GIGeneric))
	{
		return FRnd();
	}
	const UMyGameInstance* GI = World->GetGameInstance<UMyGameInstance>();
	if(!GI)
	{
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
	return GetRnd(GS, GI);
}

void UMyState::WithPlayerUI(const UObject* Object, const FLocalPlayerContext& LPC, const std::function<void(FPlayerUI&)> Func)
{
	AMyPlayerState* PS = LPC.GetPlayerState<AMyPlayerState>();
	if(!PS)
	{
		const APlayerState* PSGeneric = LPC.GetPlayerState<APlayerState>();
		UE_LOG
			( LogPlayerController
			, Error
			, TEXT("WithPlayerUI: %s: AMyPlayerState null; APlayerState: %s")
			, *Object->GetFullName()
			, PSGeneric ? *PSGeneric->GetFullName() : TEXT("null")
			)
		return;
	}
	Func(PS->PlayerUI);
}

void UMyState::WithInstanceUI(const UObject* Object, const std::function<void(FInstanceUI&)> Func)
{
	const UWorld* World = Object->GetWorld();
	UMyGameInstance* GI = World->GetGameInstance<UMyGameInstance>();
	if(!GI)
	{
		const UGameInstance* GIGeneric = World->GetGameInstance();
		UE_LOG
		( LogGameState
		, Error
		, TEXT("UMyState::WithInstanceUI: %s: UMyGameInstance null; UGameInstance: %s")
		, *Object->GetFullName()
		, GIGeneric ? *GIGeneric->GetFullName() : TEXT("null")
		)
		return;
	}
	Func(GI->InstanceUI);
}
