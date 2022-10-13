// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/MyHUDMenu.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Lib/FunctionLib.h"
#include "Lib/UStateLib.h"
#include "Menu/UW_HostGame.h"
#include "Menu/UW_LoadingScreen.h"
#include "Menu/UW_MenuMain.h"
#include "Menu/UW_MenuMultiPlayer.h"
#include "Menu/UW_MenuSolo.h"
#include "Menu/UW_Message.h"
#include "Menu/UW_ServerList.h"
#include "Menu/UW_ServerRow.h"
#include "Modes/MyGameInstance.h"
#include "Modes/MyGISubsystem.h"
#include "Modes/MyPlayerState.h"

#define LOCTEXT_NAMESPACE "Menu"

void AMyHUDMenu::BeginPlay()
{
	Super::BeginPlay();

	UGameInstance* GI = GetGameInstance();

	// set up main menu

	if(!WidgetMenuMainClass)
	{
		UE_LOG(LogSlate, Error, TEXT("%s: WidgetMainMenuClass null"), *GetFullName())
		return;
	}

	WidgetMenuMain = CreateWidget<UUW_MenuMain>(GI, WidgetMenuMainClass, FName(TEXT("Main Menu")));
	WidgetMenuMain->AddToViewport(1);

	// set up solo menu

	if(!WidgetMenuSoloClass)
	{
		UE_LOG(LogSlate, Error, TEXT("%s: WidgetMenuMultiplayerClass null"), *GetFullName())
		return;
	}
	WidgetMenuSolo = CreateWidget<UUW_MenuSolo>(GI, WidgetMenuSoloClass, FName(TEXT("Menu Solo")));
	WidgetMenuSolo->SetVisibility(ESlateVisibility::Collapsed);
	WidgetMenuSolo->AddToViewport(1);

	// set up multiplayer menu

	if(!WidgetMenuMultiplayerClass)
	{
		UE_LOG(LogSlate, Error, TEXT("%s: WidgetMenuMultiplayerClass null"), *GetFullName())
		return;
	}
	WidgetMenuMultiplayer = CreateWidget<UUW_MenuMultiPlayer>(GI, WidgetMenuMultiplayerClass, FName(TEXT("Menu Multiplayer")));
	WidgetMenuMultiplayer->SetVisibility(ESlateVisibility::Collapsed);
	WidgetMenuMultiplayer->AddToViewport(1);

	// set up menu server list
	
	if(!IsValid(WidgetServerListClass))
	{
		UE_LOG(LogSlate, Error, TEXT("%s: WidgetServerListClass null"), *GetFullName())
		return;
	}
	if(!IsValid(WidgetServerRowClass))
	{
		UE_LOG(LogSlate, Error, TEXT("%s: WidgetServerRowClass null"), *GetFullName())
	}

	WidgetServerList = CreateWidget<UUW_ServerList>(GI, WidgetServerListClass, FName(TEXT("Server List")));
	WidgetServerList->SetVisibility(ESlateVisibility::Collapsed);
	WidgetServerList->AddToViewport(1);

	// host game
	
	if(!IsValid(WidgetHostGameClass))
	{
		UE_LOG(LogSlate, Error, TEXT("%s: WidgetHostGameClass null"), *GetFullName())
		return;
	}
	WidgetHostGame = CreateWidget<UUW_HostGame>(GI, WidgetHostGameClass, FName(TEXT("Host Game")));
	WidgetHostGame->SetVisibility(ESlateVisibility::Collapsed);
	WidgetHostGame->AddToViewport(1);

	// loading screen
	
	if(!IsValid(WidgetLoadingScreenClass))
	{
		UE_LOG(LogSlate, Error, TEXT("%s: UMGWidgetLoadingScreenClass null"), *GetFullName())
		return;
	}
	WidgetLoadingScreen = CreateWidget<UUW_LoadingScreen>(GI, WidgetLoadingScreenClass, FName(TEXT("Loading Screen")));
	WidgetLoadingScreen->SetVisibility(ESlateVisibility::Collapsed);
	WidgetLoadingScreen->AddToViewport(1);

	if(!IsValid(WidgetMessageClass))
	{
		UE_LOG(LogSlate, Error, TEXT("%s: UMGWidgetMessageClass null"), *GetFullName())
		return;
	}
	WidgetMessage = CreateWidget<UUW_Message>(GI, WidgetMessageClass, FName(TEXT("Message")));
	WidgetMessage->SetVisibility(ESlateVisibility::Collapsed);
	WidgetMessage->AddToViewport(1);
}

void AMyHUDMenu::ServerListShow()
{
	HideViewportParentWidgets();
	WidgetServerList->SetVisibility(ESlateVisibility::Visible);
	ServerListRefresh();
}

void AMyHUDMenu::ServerListRefresh()
{
	WidgetServerList->SetStatusMessage(LOCTEXT("SearchingSessions", "Searching for sessions ..."));
	WidgetServerList->SetBtnRefreshEnabled(false);
	GetGameInstance()->GetSubsystem<UMyGISubsystem>()->FindSessions
		( GetOwningPlayerController()->GetPlayerState<AMyPlayerState>()->GetUniqueId()
		, [this] (bool bSuccess)
	{
		WidgetServerList->SetBtnRefreshEnabled(true);
		if(bSuccess)
		{
			ServerListUpdate();
		}
		else
		{
			MessageShow
				( LOCTEXT("ErrorSessionSearch", "Error when trying to search for sessions")
				, [this] () { MenuMainShow(); }
				);
		}
	});
}

void AMyHUDMenu::HostGameShow()
{
	HideViewportParentWidgets();
	WidgetHostGame->SetVisibility(ESlateVisibility::Visible);

	const TObjectPtr<UMyGameInstance> GI = GetGameInstance<UMyGameInstance>();
	auto [_RndGen, _Poisson, Random] = UStateLib::GetRndUnsafe(this);
	GI->SessionConfig.CustomName = UFunctionLib::Satellites[static_cast<int>(Random.FRand() * UFunctionLib::LengthSatellites)];
	const IOnlineSubsystem* SS = IOnlineSubsystem::Get
		( GI->SessionConfig.bEnableLAN ? FName(TEXT("NULL")) : FName(TEXT("EOS"))
		);
	
	WidgetHostGame->SetInfo (FText::Format
		(LOCTEXT("textinfo", "Sub: {0} | Online Service: {1} | Social Platform: {2}")
		, FText::FromName(SS->GetSubsystemName())
		, SS->GetOnlineServiceName()
		, SS->GetSocialPlatformName()
		));
}

void AMyHUDMenu::MenuMultiplayerShow()
{
	HideViewportParentWidgets();
	WidgetMenuMultiplayer->SetVisibility(ESlateVisibility::Visible);
}

void AMyHUDMenu::ServerListUpdate()
{
	const TObjectPtr<UMyGISubsystem> GISub = GetGameInstance<UMyGameInstance>()->GetSubsystem<UMyGISubsystem>();
	const TArray<FOnlineSessionSearchResult> Results = GISub->GetSearchResult();

	WidgetServerList->ClearServerRows();
	if(Results.IsEmpty())
	{
		WidgetServerList->SetStatusMessage(LOCTEXT("NoSessionsFound", "no sessions found"));
		return;
	}
	else
	{
		WidgetServerList->HideStatusMessage();
	}

	UGameInstance* GI = GetGameInstance();
	for(const FOnlineSessionSearchResult& Result : Results)
	{
		UUW_ServerRow* Row = CreateWidget<UUW_ServerRow>(GI, WidgetServerRowClass);

		Row->SetPing(Result.PingInMs);
		FString CustomName;
		Result.Session.SessionSettings.Get(SETTING_CUSTOMNAME, CustomName);
		Row->SetServerName(FText::FromString(CustomName));
		
		const int NumOpenPrivateConnections = Result.Session.NumOpenPrivateConnections;
		const int NumPrivateConnections = Result.Session.SessionSettings.NumPrivateConnections;
		const int NumOpenPublicConnections = Result.Session.NumOpenPublicConnections;
		const int NumPublicConnections = Result.Session.SessionSettings.NumPublicConnections;
		int MaxNumPlayers = 0;
		int NumPlayers = 0;
		if(NumPrivateConnections == 0)
		{
			// public session
			MaxNumPlayers = NumPublicConnections;
			NumPlayers = MaxNumPlayers - NumOpenPublicConnections;
		}
		else if(NumPublicConnections == 0)
		{
			// private session
			MaxNumPlayers = NumPrivateConnections;
			NumPlayers = MaxNumPlayers - NumOpenPrivateConnections;
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
		Row->SetPlayerNumbers(NumPlayers, MaxNumPlayers);
		WidgetServerList->AddServerRow(Row);
	}
}

void AMyHUDMenu::MenuMainShow()
{
	HideViewportParentWidgets();
	WidgetMenuMain->SetVisibility(ESlateVisibility::Visible);
}

void AMyHUDMenu::MenuSoloShow()
{
	HideViewportParentWidgets();
	WidgetMenuSolo->SetVisibility(ESlateVisibility::Visible);
}

void AMyHUDMenu::LoadingScreenShow(const FText& StrMessage, TFunctionRef<void()> GobackFunc)
{
	HideViewportParentWidgets();
	WidgetLoadingScreen->SetMessage(StrMessage);
	WidgetLoadingScreen->SetGobackFunc(GobackFunc);
	WidgetLoadingScreen->SetVisibility(ESlateVisibility::Visible);
}

void AMyHUDMenu::MessageShow(const FText& StrMessage, TFunctionRef<void()> FuncGoBack)
{
	HideViewportParentWidgets();
	WidgetMessage->SetMessage(StrMessage);
	WidgetMessage->SetGobackFunc(FuncGoBack);
	WidgetMessage->SetVisibility(ESlateVisibility::Visible);
}

#undef LOCTEXT_NAMESPACE