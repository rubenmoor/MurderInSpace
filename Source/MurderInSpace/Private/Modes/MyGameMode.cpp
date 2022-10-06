// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MyGameMode.h"

#include "Modes/MyGameInstance.h"
#include "Lib/UStateLib.h"

void AMyGameMode::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	const auto GI = GetGameInstance<UMyGameInstance>();
	if(!GI)
	{
		UE_LOG
			( LogGameMode
			, Warning
			, TEXT("%s: GI null; assuming IN EDITOR")
			, *GetFullName()
			)
		return;
	}
	UStateLib::SetInstanceState(GI, EInstanceState::InGame);
}
