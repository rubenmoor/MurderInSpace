// Fill out your copyright notice in the Description page of Project Settings.


#include "MyHUDMenu.h"

#include "MyGameInstance.h"
#include "Kismet/KismetSystemLibrary.h"

void AMyHUDMenu::BeginPlay()
{
	if(!UMGWidgetServerList)
	{
		UE_LOG(LogSlate, Error, TEXT("%s: UMGWidgetServerList null"), *GetFullName())
		return;
	}
	if(!UMGWidgetMenuInGame)
	{
		UE_LOG(LogSlate, Error, TEXT("%s: UMGWidgetMenuInGame null"), *GetFullName())
		return;
	}
	Super::BeginPlay();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AMyHUDMenu::HandleBtnFindServerClicked()
{
	// TODO: get server list
	GetGameInstance<UMyGameInstance>()->ShowServers();
	UMGWidget->RemoveFromViewport();
	// TODO: pass list of servers maybe?
	SetWidgetServerList();
}

void AMyHUDMenu::SetWidgetToDefault()
{
	Super::SetWidgetToDefault();

	FindOrFail<UButton>(FName(TEXT("BtnStart")))->OnClicked.AddDynamic
		( GetGameInstance<UMyGameInstance>()
		, &UMyGameInstance::HostGame
		);
	
	FindOrFail<UButton>(FName(TEXT("FindServer")))->OnClicked.AddDynamic
		( this
		, &AMyHUDMenu::HandleBtnFindServerClicked
		);
	
	FindOrFail<UButton>(FName(TEXT("BtnQuit")))->OnClicked.AddDynamic
		( GetGameInstance<UMyGameInstance>()
		, &UMyGameInstance::QuitGame
		);
}

void AMyHUDMenu::SetWidgetServerList()
{
	UMGWidget->RemoveFromViewport();
	UMGWidget = CreateWidget<UUserWidget>(GetOwningPlayerController(), UMGWidgetServerList);
	UMGWidget->AddToViewport();

	FindOrFail<UButton>(FName(TEXT("BtnBack")))->OnClicked.AddDynamic(this, &AMyHUDMenu::SetWidgetToDefault);
	// TODO: refresh on clicked
	// TODO: server list: server row on clicked
}

void AMyHUDMenu::SetWidgetMenuInGame()
{
	UMGWidget->RemoveFromViewport();
	UMGWidget = CreateWidget<UUserWidget>(GetOwningPlayerController(), UMGWidgetMenuInGame);
	UMGWidget->AddToViewport();
	
	FindOrFail<UButton>(FName(TEXT("BtnLeave")))->OnClicked.AddDynamic(this, &AMyHUDMenu::Leave);
	FindOrFail<UButton>(FName(TEXT("Resume")))->OnClicked.AddDynamic
		( GetGameInstance<UMyGameInstance>()
		, &UMyGameInstance::SetHUD
		);
	FindOrFail<UButton>(FName(TEXT("BtnQuit")))->OnClicked.AddDynamic
		(GetGameInstance<UMyGameInstance>()
		, &UMyGameInstance::QuitGame
		);
}

void AMyHUDMenu::Leave()
{
	UMGWidget->RemoveFromViewport();
	SetWidgetToDefault();
	GetGameInstance<UMyGameInstance>()->DestroySession();
}
