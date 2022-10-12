// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MyPlayerState.h"

#include "OnlineSubsystemUtils.h"

void AMyPlayerState::BeginPlay()
{
	Super::BeginPlay();

	UMyGameInstance* GI = GetGameInstance<UMyGameInstance>();
	const IOnlineIdentityPtr OSSIdentity = Online::GetIdentityInterfaceChecked(FName(TEXT("EOS")));
	const FUniqueNetIdRepl UniqueNetIdRepl = GetUniqueId();
	if(UniqueNetIdRepl.IsValid())
	{
		GI->bLoggedIn = OSSIdentity->GetLoginStatus(*UniqueNetIdRepl.GetUniqueNetId()) == ELoginStatus::LoggedIn;
	}
	else
	{
		const bool loggedIn = OSSIdentity->GetLoginStatus(GetPlayerId()) == ELoginStatus::LoggedIn;
		GI->bLoggedIn = loggedIn;
		if(loggedIn)
		{
			SetUniqueId(FUniqueNetIdRepl(OSSIdentity->GetUniquePlayerId(GetPlayerId())));
		}
		else
		{
			SetUniqueId(FUniqueNetIdRepl(Online::GetIdentityInterfaceChecked(FName(TEXT("NULL")))->CreateUniquePlayerId
				(FGuid::NewGuid().ToString() )));
		}
	}
}

