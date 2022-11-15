// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimationCompression.h"
#include "MyHUDBase.h"
#include "Menu/UW_HostGame.h"
#include "Menu/UW_LoadingScreen.h"
#include "Menu/UW_MenuMain.h"
#include "Menu/UW_MenuMultiPlayer.h"
#include "Menu/UW_MenuSolo.h"
#include "Menu/UW_Message.h"
#include "Menu/UW_ServerList.h"
#include "Menu/UW_ServerRow.h"

#include "MyHUDMenu.generated.h"


/**
 * 
 */
UCLASS()
class MURDERINSPACE_API AMyHUDMenu : public AMyHUDBase
{
	GENERATED_BODY()

	friend class UMyGameInstance;
protected:

	// main menu
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMG Widget Classes")
	TSubclassOf<UUW_MenuMain> WidgetMenuMainClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UMG Widgets")
	TObjectPtr<UUW_MenuMain> WidgetMenuMain;

	// play solo menu
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMG Widget Classes")
	TSubclassOf<UUW_MenuSolo> WidgetMenuSoloClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UMG Widgets")
	TObjectPtr<UUW_MenuSolo> WidgetMenuSolo;
	
	// multiplayer menu
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMG Widget Classes")
	TSubclassOf<UUW_MenuMultiPlayer> WidgetMenuMultiplayerClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UMG Widgets")
	TObjectPtr<UUW_MenuMultiPlayer> WidgetMenuMultiplayer;

	// server list
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMG Widget Classes")
	TSubclassOf<UUW_ServerList> WidgetServerListClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UMG Widgets")
	TObjectPtr<UUW_ServerList> WidgetServerList;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMG Widget Classes")
	TSubclassOf<UUW_ServerRow> WidgetServerRowClass;

	// host game session config
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMG Widget Classes")
	TSubclassOf<UUW_HostGame> WidgetHostGameClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UMG Widgets")
	TObjectPtr<UUW_HostGame> WidgetHostGame;
	

	// loading screen
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMG Widget Classes")
	TSubclassOf<UUW_LoadingScreen> WidgetLoadingScreenClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UMG Widgets")
	TObjectPtr<UUW_LoadingScreen> WidgetLoadingScreen;

	// message
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMG Widget Classes")
	TSubclassOf<UUW_Message> WidgetMessageClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UMG Widgets")
	TObjectPtr<UUW_Message> WidgetMessage;
	
	// event handlers

	virtual void BeginPlay() override;

	// debugging

	void Shout() { UE_LOG(LogSlate, Warning, TEXT("Shouting!")); }
public:
	// public methods
	
	UFUNCTION(BlueprintCallable)
	void MenuMainShow();

	UFUNCTION(BlueprintCallable)
	void MenuSoloShow();

	UFUNCTION(BlueprintCallable)
	void MenuMultiplayerShow();

	UFUNCTION(BlueprintCallable)
	void ServerListShow();

	UFUNCTION(BlueprintCallable)
	void ServerListRefresh();
	
	UFUNCTION(BlueprintCallable)
	void HostGameShow();

	void MessageShow(const FText& StrMessage, std::function<void()> FuncGoBack);
	
	// update the server list in the HUD
	// to be called when a session search has returned new results
	UFUNCTION(BlueprintCallable)
	void ServerListUpdate();
	
	void LoadingScreenShow(const FText& StrMessage, std::function<void()> GobackFunc);
};
