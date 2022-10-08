// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/MyHUDMenu.h"

#include "OnlineSessionSettings.h"
#include "Components/ScrollBox.h"
#include "HUD/MyCommonButton.h"
#include "Modes/MyGameInstance.h"
#include "Modes/MyGISubsystem.h"

#define LOCTEXT_NAMESPACE "Menu"

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
			// TODO: get value for session config from menu in the UI
			const FSessionConfig SessionConfig =
				{TEXT("My awesome session")
				, 4
				, false
				, true
				};
			GI->HostGame(SessionConfig);
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
		UE_LOG(LogSlate, Error, TEXT("%s: UMGWidgetServerListClass null"), *GetFullName())
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

	if(!WidgetLoadingScreenClass)
	{
		UE_LOG(LogSlate, Error, TEXT("%s: UMGWidgetLoadingScreenClass null"), *GetFullName())
		return;
	}
	WidgetLoadingScreen = CreateWidget(GetWorld(), WidgetLoadingScreenClass, FName(TEXT("Loading Screen")));
	WidgetLoadingScreen->SetVisibility(ESlateVisibility::Collapsed);
	WidgetLoadingScreen->AddToViewport(1);

	if(!WidgetMessageClass)
	{
		UE_LOG(LogSlate, Error, TEXT("%s: UMGWidgetMessageClass null"), *GetFullName())
		return;
	}
	WidgetMessage = CreateWidget(GetWorld(), WidgetMessageClass, FName(TEXT("Message")));
	WidgetMessage->SetVisibility(ESlateVisibility::Collapsed);
	WidgetMessage->AddToViewport(1);
}

void AMyHUDMenu::ServerListShow()
{
	HideViewportParentWidgets();
	WidgetServerList->SetVisibility(ESlateVisibility::Visible);
}

void AMyHUDMenu::ServerListRefresh(TArray<FOnlineSessionSearchResult> Results)
{
	WithWidget<UScrollBox>(WidgetServerList, FName(TEXT("ScrollServers")), [this, Results] (TObjectPtr<UScrollBox> ScrollServers)
	{
		for(const FOnlineSessionSearchResult& Result : Results)
		{
			auto Row = CreateWidget(GetWorld(), WidgetServerRowClass);
			
			WithWidget<UCommonTextBlock>(Row, FName(TEXT("TextPing")), [Result] (TObjectPtr<UCommonTextBlock> TextPing)
			{
				TextPing->SetText(FText::Format(LOCTEXT("ping", "%d"), Result.PingInMs));
			});
			WithWidget<UCommonTextBlock>(Row, FName(TEXT("TextServerName")), [Result] (TObjectPtr<UCommonTextBlock> TextServerName)
			{
				FString CustomName;
				Result.Session.SessionSettings.Get(SETTING_CUSTOMNAME, CustomName);
				TextServerName->SetText(FText::FromString(CustomName));
			});
			WithWidget<UCommonTextBlock>(Row, FName(TEXT("TextPlayerCount")), [this, Result] (TObjectPtr<UCommonTextBlock> TextPlayerCount)
			{
				const int NumOpenPrivateConnections = Result.Session.NumOpenPrivateConnections;
				const int NumPrivateConnections = Result.Session.SessionSettings.NumPrivateConnections;
				const int NumOpenPublicConnections = Result.Session.NumOpenPublicConnections;
				const int NumPublicConnections = Result.Session.SessionSettings.NumPublicConnections;
				int NumMaxPlayers = 0;
				int NumPlayers = 0;
				if(NumPrivateConnections == 0)
				{
					// public session
					NumMaxPlayers = NumPublicConnections;
					NumPlayers = NumMaxPlayers - NumOpenPublicConnections;
				}
				else if(NumPublicConnections == 0)
				{
					// private session
					NumMaxPlayers = NumPrivateConnections;
					NumPlayers = NumMaxPlayers - NumOpenPrivateConnections;
				}
				else
				{
					UE_LOG
						( LogNet
						, Error
						, TEXT("%s: couldn't determine whether session is private or public")
						, *GetFullName()
						)
				}
				TextPlayerCount->SetText(FText::Format(LOCTEXT("Playercount", "%d / %d"), NumPlayers, NumMaxPlayers));
			});
			ScrollServers->AddChild(Row);
		}
	});
}

void AMyHUDMenu::MainMenuShow()
{
	HideViewportParentWidgets();
	WidgetMainMenu->SetVisibility(ESlateVisibility::Visible);
}

void AMyHUDMenu::LoadingScreenShow(FText StrMessage)
{
	HideViewportParentWidgets();
	WithWidget<UCommonTextBlock>(WidgetLoadingScreen, FName(TEXT("TextMessage")), [&StrMessage] (TObjectPtr<UCommonTextBlock> TextMessage)
	{
		TextMessage->SetText(StrMessage);
	});
	WidgetLoadingScreen->SetVisibility(ESlateVisibility::Visible);
}

void AMyHUDMenu::MessageShow(FText StrMessage, TFunctionRef<void()> FuncGoBack)
{
	HideViewportParentWidgets();
	WithWidget<UCommonTextBlock>(WidgetMessage, FName(TEXT("TextMessage")), [&StrMessage] (TObjectPtr<UCommonTextBlock> TextMessage)
	{
		TextMessage->SetText(StrMessage);
	});
	WithWidget<UMyCommonButton>(WidgetMessage, FName(TEXT("BtnBack")), [this, FuncGoBack] (TObjectPtr<UMyCommonButton> Button)
	{
		DHMessageShowGoBack = Button->OnClicked().AddLambda([this, FuncGoBack, Button] ()
		{
			FuncGoBack();
			Button->OnClicked().Remove(DHMessageShowGoBack);
		});
	});
	WidgetMessage->SetVisibility(ESlateVisibility::Visible);
}
# undef LOCTEXT_NAMESPACE
