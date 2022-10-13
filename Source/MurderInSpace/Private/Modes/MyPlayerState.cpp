// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MyPlayerState.h"

#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineIdentityInterface.h"

void AMyPlayerState::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetPlayerController();
	UMyGameInstance* GI = GetGameInstance<UMyGameInstance>();
	
	// for the identity interface, SubsystemName == "NULL" doesn't make sense, I believe, as there
	// is nor identity provider in the NULL/LAN online subystem
	const IOnlineIdentityPtr OSSIdentity = Online::GetIdentityInterfaceChecked(FName(TEXT("EOS")));
	
	// valid only for player controlled player state
	// player states of remote players and/or AI may exist
	if(IsValid(PC))
	{
		if(!GetUniqueId().IsValid())
		{
			UE_LOG
				( LogNet
				, Warning
				, TEXT("%s: unique net id wasn't set, getting from local player ...")
				, *GetFullName()
				)
			SetUniqueId(PC->GetLocalPlayer()->GetPreferredUniqueNetId());
			if(!GetUniqueId().IsValid())
			{
				UE_LOG
					( LogNet
					, Warning
					, TEXT("%s: unique net id of local player wasn't set, either, getting from online identity interface via local user index ...")
					, *GetFullName()
					)
				SetUniqueId(FUniqueNetIdRepl(OSSIdentity->GetUniquePlayerId(PC->GetLocalPlayer()->GetIndexInGameInstance())));
				if(!GetUniqueId().IsValid())
				{
					UE_LOG
						( LogNet
						, Warning
						, TEXT("%s: unique net id from online identity interface null, creating new random ...")
						, *GetFullName()
						)
					SetUniqueId(FUniqueNetIdRepl(OSSIdentity->CreateUniquePlayerId(FGuid::NewGuid().ToString() )));
				}
			}
		}
		
		GI->bLoggedIn = OSSIdentity->GetLoginStatus(*GetUniqueId()) == ELoginStatus::LoggedIn;
		UE_LOG
			( LogNet
			, Display
			, TEXT("%s: ... unique net id: %s; player is%s logged in")
			, *GetFullName()
			, *GetUniqueId().ToString()
			, GI->bLoggedIn ? TEXT("") : TEXT(" not")
			)
	}
}

void AMyPlayerState::OnSetUniqueId()
{
	Super::OnSetUniqueId();
	const APlayerController* PC = GetPlayerController();
	if(IsValid(PC))
	{
		PC->GetLocalPlayer()->SetCachedUniqueNetId(GetUniqueId());
	}
}
