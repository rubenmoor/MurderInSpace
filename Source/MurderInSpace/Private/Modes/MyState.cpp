// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MyState.h"

#include "GameplayTagsManager.h"
#include "Modes/MyGameInstance.h"
#include "Modes/MyGameState.h"
#include "Modes/MyPlayerState.h"

DEFINE_LOG_CATEGORY(LogMyGame);

float UMyState::GetInitialAngularVelocity(FRnd Rnd)
{
	return Rnd.Poisson(Rnd.RndGen) / 1.e3;
}

void UMyState::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
    UGameplayTagsManager& GTM = UGameplayTagsManager::Get();

	InputTags.SetNumZeroed(static_cast<uint8>(EInputAction::Last) + 1);
	
    InputTags[static_cast<uint8>(EInputAction::AccelerateBeginEnd     )] = GTM.AddNativeGameplayTag(FName(TEXT("InputAction.AccelerateBeginEnd")));
    
    InputTags[static_cast<uint8>(EInputAction::IngameMenuToggle       )] = GTM.AddNativeGameplayTag(FName(TEXT("InputAction.IngameMenuToggle")));
    InputTags[static_cast<uint8>(EInputAction::MyTrajectoryShowHide   )] = GTM.AddNativeGameplayTag(FName(TEXT("InputAction.MyTrajectoryShowHide")));
    InputTags[static_cast<uint8>(EInputAction::AllTrajectoriesShowHide)] = GTM.AddNativeGameplayTag(FName(TEXT("InputAction.AllTrajectoriesShowHide")));
    InputTags[static_cast<uint8>(EInputAction::MyTrajectoryToggle     )] = GTM.AddNativeGameplayTag(FName(TEXT("InputAction.MyTrajectoryToggle")));
    
    GTM.DoneAddingNativeTags();
}

FPhysics UMyState::GetPhysics(const AMyGameState* GS)
{
	return GS->Physics;
}

FPhysics UMyState::GetPhysicsAny(const UObject* Object)
{
	const UWorld* World = Object->GetWorld();
	const AGameState* GSGeneric = World->GetGameState<AGameState>();
	if(!IsValid(GSGeneric))
	{
		return PhysicsEditorDefault;
	}
	const AMyGameState* GS = World->GetGameState<AMyGameState>();
	if(!GS)
	{
		UE_LOG
			( LogGameState
			, Error
			, TEXT("UMyState::GetPhysicsAny: %s: AMyGameState null; AGameState: %s")
			, *Object->GetFullName()
			, *GSGeneric->GetFullName()
			)
		return PhysicsEditorDefault;
	}
	return GetPhysics(GS);
}

FPlayerUI UMyState::GetPlayerUI(const AMyPlayerState* PS)
{
	return PS->PlayerUI;
}

FPlayerUI UMyState::GetPlayerUIAny(const UObject* Object, const FLocalPlayerContext& LPC)
{
	const APlayerState* PSGeneric = LPC.GetPlayerState<APlayerState>();
	if(!IsValid(PSGeneric))
	{
		return PlayerUIEditorDefault;
	}
	const AMyPlayerState* PS = LPC.GetPlayerState<AMyPlayerState>();
	if(!PS)
	{
		UE_LOG
			( LogPlayerController
			, Error
			, TEXT("UStateLib::GetPlayerUIUnsafe: %s: AMyPlayerState null; APlayerState: %s")
			, *Object->GetFullName()
			, *PSGeneric->GetFullName()
			)
		return PlayerUIEditorDefault;
	}
	return GetPlayerUI(PS);
}

FInstanceUI UMyState::GetInstanceUI(const UMyGameInstance* GI)
{
	return GI->InstanceUI;
}

FInstanceUI UMyState::GetInstanceUIAny(const UObject* Object)
{
	const UWorld* World = Object->GetWorld();
	const UGameInstance* GIGeneric = World->GetGameInstance();
	if(!IsValid(GIGeneric))
	{
		return InstanceUIEditorDefault;
	}
	const UMyGameInstance* GI = World->GetGameInstance<UMyGameInstance>();
	if(!GI)
	{
		UE_LOG
		( LogGameState
		, Error
		, TEXT("UStateLib::GetInstanceUIUnsafe: %s: UMyGameInstance null; UGameInstance: %s")
		, *Object->GetFullName()
		, *GIGeneric->GetFullName()
		)
		return InstanceUIEditorDefault;
	}
	return GetInstanceUI(GI);
}

FRnd UMyState::GetRnd(const AMyGameState* GS, const UMyGameInstance* GI)
{
	return FRnd
	{ GS->RndGen
		, GS->Poisson
		, GI->Random
	};
}

FRnd UMyState::GetRndAny(const UObject* Object)
{
	const UWorld* World = Object->GetWorld();
	const UGameInstance* GIGeneric = World->GetGameInstance();
	const AGameState* GSGeneric = World->GetGameState<AGameState>();
	if(!IsValid(GIGeneric) || !IsValid(GSGeneric))
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
			, *GIGeneric->GetFullName()
			)
		return FRnd();
	}
	
	const AMyGameState* GS = World->GetGameState<AMyGameState>();
	if(!GS)
	{
		UE_LOG
			( LogGameState
			, Error
			, TEXT("UStateLib::GetRndUnsafe: %s: AMyGameState null; AGameState: %s")
			, *Object->GetFullName()
			, *GSGeneric->GetFullName()
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
