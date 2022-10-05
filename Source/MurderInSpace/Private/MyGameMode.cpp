// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameMode.h"

#include "MyGameInstance.h"
#include "UStateLib.h"

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
