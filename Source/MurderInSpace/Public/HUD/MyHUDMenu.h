// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyHUDBase.h"

#include "MyHUDMenu.generated.h"

class UUW_HostGame;
class UUW_Message;
class UUW_LoadingScreen;
class UUW_MenuMain;
class UUW_MenuSolo;
class UUW_MenuMultiPlayer;
class UUW_ServerList;

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
	TSubclassOf<UUserWidget> WidgetMenuMainClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UMG Widgets")
	TObjectPtr<UUW_MenuMain> WidgetMenuMain;

	// play solo menu
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMG Widget Classes")
	TSubclassOf<UUserWidget> WidgetMenuSoloClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UMG Widgets")
	TObjectPtr<UUW_MenuSolo> WidgetMenuSolo;
	
	// multiplayer menu
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMG Widget Classes")
	TSubclassOf<UUserWidget> WidgetMenuMultiplayerClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UMG Widgets")
	TObjectPtr<UUW_MenuMultiPlayer> WidgetMenuMultiplayer;

	// server list
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMG Widget Classes")
	TSubclassOf<UUserWidget> WidgetServerListClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UMG Widgets")
	TObjectPtr<UUW_ServerList> WidgetServerList;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMG Widget Classes")
	TSubclassOf<UUserWidget> WidgetServerRowClass;

	// host game session config
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMG Widget Classes")
	TSubclassOf<UUserWidget> WidgetHostGameClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UMG Widgets")
	TObjectPtr<UUW_HostGame> WidgetHostGame;
	

	// loading screen
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMG Widget Classes")
	TSubclassOf<UUserWidget> WidgetLoadingScreenClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UMG Widgets")
	TObjectPtr<UUW_LoadingScreen> WidgetLoadingScreen;

	// message
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMG Widget Classes")
	TSubclassOf<UUserWidget> WidgetMessageClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UMG Widgets")
	TObjectPtr<UUW_Message> WidgetMessage;
	
	// event handlers

	virtual void BeginPlay() override;

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
	void HostGameShow();

	void MessageShow(const FText& StrMessage, TFunctionRef<void()> FuncGoBack);
	
	// update the server list in the HUD
	// to be called when a session search has returned new results
	UFUNCTION(BlueprintCallable)
	void ServerListUpdate();
	
	void LoadingScreenShow(const FText& StrMessage, TFunctionRef<void()> GobackFunc);
};
