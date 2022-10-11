// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MyPlayerState.h"

#include "OnlineSubsystemUtils.h"

void AMyPlayerState::BeginPlay()
{
	Super::BeginPlay();

	const auto StrGuid = FGuid::NewGuid().ToString();
	SetUniqueId(FUniqueNetIdRepl(Online::GetSubsystem(GetWorld(), FName(TEXT("NULL")))->GetIdentityInterface()->CreateUniquePlayerId(StrGuid)));
}
