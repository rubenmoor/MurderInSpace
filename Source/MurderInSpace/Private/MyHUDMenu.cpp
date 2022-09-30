// Fill out your copyright notice in the Description page of Project Settings.


#include "MyHUDMenu.h"

#include "MyGameInstance.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"

void AMyHUDMenu::BeginPlay()
{
	Super::BeginPlay();

	const auto PC = GetOwningPlayerController();
	if(!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: BeginPlay: no player controller, disabling tick"), *GetFullName())
		SetActorTickEnabled(false);
		return;
	}

	const UMyGameInstance* GI = GetGameInstance<UMyGameInstance>();
	
	// set up main menu

	if(!WidgetMainMenuClass)
	{
		UE_LOG(LogSlate, Error, TEXT("%s: WidgetMainMenuClass null"), *GetFullName())
		return;
	}

	WidgetMainMenu = CreateWidget<UUserWidget>(PC, WidgetMainMenuClass);
	WidgetMainMenu->AddToViewport();
	
	FindOrFail<UButton>(WidgetMainMenu, FName(TEXT("BtnStart"  )));
	//->OnClicked.AddUniqueDynamic(GI, &UMyGameInstance::HostGame         );
	//FindOrFail<UButton>(WidgetMainMenu, FName(TEXT("BtnStart"  )))->OnClicked.AddUniqueDynamic(GI, &UMyGameInstance::HostGame         );
	///FindOrFail<UButton>(WidgetMainMenu, FName(TEXT("FindServer")))->OnClicked.AddUniqueDynamic(GI, &UMyGameInstance::GotoInMenuServers);
	//FindOrFail<UButton>(WidgetMainMenu, FName(TEXT("BtnQuit"   )))->OnClicked.AddUniqueDynamic(GI, &UMyGameInstance::QuitGame         );

	// set up menu server list
	
	if(!WidgetServerListClass)
	{
		UE_LOG(LogSlate, Error, TEXT("%s: UMGWidgetServerList null"), *GetFullName())
		return;
	}

	WidgetServerList = CreateWidget<UUserWidget>(PC, WidgetServerListClass);
	WidgetServerList->SetVisibility(ESlateVisibility::Collapsed);
	WidgetServerList->AddToViewport();
	
	FindOrFail<UButton>(WidgetServerList, FName(TEXT("BtnBack")))->OnClicked.AddDynamic(GI, &UMyGameInstance::GotoInMenuMain);
	// TODO: refresh on clicked
	// TODO: server list: server row on clicked
}

void AMyHUDMenu::ServerListShow()
{
	HideViewportParentWidgets();
	WidgetServerList->SetVisibility(ESlateVisibility::Visible);
}

void AMyHUDMenu::MainMenuShow()
{
	HideViewportParentWidgets();
	WidgetMainMenu->SetVisibility(ESlateVisibility::Visible);
}

void AMyHUDMenu::BtnStartClicked()
{
	GetGameInstance<UMyGameInstance>()->HostGame();
}
