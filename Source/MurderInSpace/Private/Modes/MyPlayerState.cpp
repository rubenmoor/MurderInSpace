// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MyPlayerState.h"

#include "OnlineSubsystemUtils.h"

void AMyPlayerState::BeginPlay()
{
	Super::BeginPlay();

	// TODO: debug with breakpoint, Log never writes
	const auto StrGuid = FGuid::NewGuid().ToString();
	SetUniqueId(FUniqueNetIdRepl(Online::GetSubsystem(GetWorld(), FName(TEXT("NULL")))->GetIdentityInterface()->CreateUniquePlayerId(StrGuid)));
	UE_LOG
		( LogPlayerManagement
		, Warning
		, TEXT("%s: Setting unique net id to %s; verifying: %s")
		, *GetFullName()
		, *StrGuid
		, *GetUniqueId().ToString()
		)
	// UE_LOG
	// 	()
	// SetUniqueId()
}
