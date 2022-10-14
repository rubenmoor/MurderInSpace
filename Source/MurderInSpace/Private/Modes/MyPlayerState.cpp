// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MyPlayerState.h"

void AMyPlayerState::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG
		( LogNet
		, Display
		, TEXT("%s: BeginPlay: Unique net id: %s")
		, *GetFullName()
		, *GetUniqueId().ToString()
		)
}
