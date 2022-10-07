// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/MyHUDMenu.h"

#include "HUD/MyCommonButton.h"
#include "Modes/MyGameInstance.h"

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

	WidgetMainMenu = CreateWidget(GetWorld(), WidgetMainMenuClass, FName(TEXT("Main Menu")));
	WidgetMainMenu->AddToViewport(1);

	WithWidget<UMyCommonButton>(WidgetMainMenu, FName(TEXT("BtnStart")), [GI] (TObjectPtr<UMyCommonButton> Button)
	{
		Button->OnClicked().AddLambda([GI] ()
		{
			GI->GotoInGame();
		});
	});
	WithWidget<UMyCommonButton>(WidgetMainMenu, FName(TEXT("BtnHost")), [GI] (TObjectPtr<UMyCommonButton> Button)
	{
		Button->OnClicked().AddLambda([GI] ()
		{
			GI->HostGame();
		});
	});
	WithWidget<UMyCommonButton>(WidgetMainMenu, FName(TEXT("BtnFindOnline")), [this] (TObjectPtr<UMyCommonButton> Button)
	{
		Button->OnClicked().AddLambda([this] () { ServerListShow(); });
	});
	// TODO: set parameter LAN
	WithWidget<UMyCommonButton>(WidgetMainMenu, FName(TEXT("BtnFindLAN")), [this] (TObjectPtr<UMyCommonButton> Button)
	{
		Button->OnClicked().AddLambda([this] () { ServerListShow(); });
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

	WidgetServerList = CreateWidget(GetWorld(), WidgetServerListClass, FName(TEXT("Server List")));
	WidgetServerList->SetVisibility(ESlateVisibility::Collapsed);
	WidgetServerList->AddToViewport(1);
	
	WithWidget<UMyCommonButton>(WidgetServerList, FName(TEXT("BtnBack")), [this] (TObjectPtr<UMyCommonButton> Button)
	{
		Button->OnClicked().AddLambda( [this] () { MainMenuShow(); });
	});
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
