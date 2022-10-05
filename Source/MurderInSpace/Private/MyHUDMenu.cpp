// Fill out your copyright notice in the Description page of Project Settings.


#include "MyHUDMenu.h"

#include "MyCommonButton.h"
#include "MyGameInstance.h"

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

	UMyGameInstance* GI = GetGameInstance<UMyGameInstance>();
	
	// set up main menu

	if(!WidgetMainMenuClass)
	{
		UE_LOG(LogSlate, Error, TEXT("%s: WidgetMainMenuClass null"), *GetFullName())
		return;
	}

	WidgetMainMenu = CreateWidget<UUserWidget>(PC, WidgetMainMenuClass);
	WidgetMainMenu->AddToViewport();

	WithWidget<UMyCommonButton>(WidgetMainMenu, FName(TEXT("BtnStart")), [GI] (TObjectPtr<UMyCommonButton> Button)
	{
		Button->OnClicked().AddLambda([GI] ()
		{
			UE_LOG(LogSlate, Warning, TEXT("Debug: BtnStart.OnClicked"))
			GI->GotoInGame();
		});
	});
	WithWidget<UMyCommonButton>(WidgetMainMenu, FName(TEXT("BtnFindOnline")), [GI] (TObjectPtr<UMyCommonButton> Button)
	{
		Button->OnClicked().AddLambda([GI] () { GI->GotoInMenuServers(); });
	});
	// TODO: set parameter LAN
	WithWidget<UMyCommonButton>(WidgetMainMenu, FName(TEXT("BtnFindLAN")), [GI] (TObjectPtr<UMyCommonButton> Button)
	{
		Button->OnClicked().AddLambda([GI] () { GI->GotoInMenuServers(); });
	});
	WithWidget<UMyCommonButton>(WidgetMainMenu, FName(TEXT("BtnQuit")), [GI] (TObjectPtr<UMyCommonButton> Button)
	{
		Button->OnClicked().AddLambda([GI] () { GI->QuitGame(); });
	});

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
