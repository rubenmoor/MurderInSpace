// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MenuGameMode.h"

#include "Modes/MyGameInstance.h"
#include "Lib/UStateLib.h"

void AMenuGameMode::PostInitializeComponents()
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
	UStateLib::SetInstanceState(GI, EInstanceState::InMenuMain);
}