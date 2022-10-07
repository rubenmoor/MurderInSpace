// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MyGISubsystem.h"
#include "OnlineSubsystemUtils.h"

UMyGISubsystem::UMyGISubsystem()
	: OnCreateSessionCompleteDelegate
		( FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::HandleGISessionCrated) )
	, OnDestroySessionCompleteDelegate
		( FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::HandleGISessionDestroyed) )
	, OnStartSessionCompleteDelegate
		( FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::HandleGISessionStarted) )
{
}

// TODO: Maybe FOnGISession...
FOnGISessionCreatedSignature UMyGISubsystem::CreateSession(int NumPublicConnections, bool bIsLanMatch)
{
	FOnGISessionCreatedSignature OnGISessionCreated;
	const IOnlineSessionPtr SI = Online::GetSessionInterface(GetWorld());
	if(!SI.IsValid())
	{
		OnGISessionCreated.Execute(false);
	}
	else
	{
		LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
		LastSessionSettings->NumPrivateConnections = 0;
		LastSessionSettings->NumPublicConnections = NumPublicConnections;
		LastSessionSettings->bAllowInvites = true;
		LastSessionSettings->bAllowJoinInProgress = true;
		LastSessionSettings->bAllowJoinViaPresence = true;
		LastSessionSettings->bAllowJoinViaPresenceFriendsOnly = true;
		LastSessionSettings->bIsDedicated = false;
		LastSessionSettings->bUsesPresence = true;
		LastSessionSettings->bIsLANMatch = bIsLanMatch;
		LastSessionSettings->bShouldAdvertise = true;

		LastSessionSettings->Set(SETTING_MAPNAME, TEXT("Space Football"), EOnlineDataAdvertisementType::ViaOnlineService);

		CreateSessionCompleteDelegateHandle = SI->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);

		const TObjectPtr<ULocalPlayer> LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
		if (!SI->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings))
		{
			SI->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

			OnGISessionCreated.Execute(false);
		}
	}
	return OnGISessionCreated;
}

void UMyGISubsystem::HandleGISessionCrated(FName SessionName, bool bSuccess)
{
	WithSessionInterface([this] (IOnlineSessionPtr SI)
	{
		SI->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	});

	OnGISessionCreated.Execute(true);
}

void UMyGISubsystem::HandleGISessionDestroyed(FName SessionName, bool bSuccess)
{
	WithSessionInterface([this] (IOnlineSessionPtr SI)
	{
		SI->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	});
	OnGISessionDestroyed.Execute(bSuccess);
}

void UMyGISubsystem::HandleGISessionStarted(FName SessionName, bool bSuccess)
{
	WithSessionInterface( [this] (IOnlineSessionPtr SI)
	{
		SI->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
	});
	OnGISessionStarted.Execute(bSuccess);
}

void UMyGISubsystem::DestroySession()
{
	const IOnlineSessionPtr SI = Online::GetSessionInterface(GetWorld());
	if (!SI.IsValid())
	{
		OnGISessionDestroyed.Execute(false);
		return;
	}

	DestroySessionCompleteDelegateHandle =
		SI->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);

	if (!SI->DestroySession(NAME_GameSession))
	{
		SI->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);

		OnGISessionDestroyed.Execute(false);
	}
}

void UMyGISubsystem::StartSession()
{
	const IOnlineSessionPtr SI = Online::GetSessionInterface(GetWorld());
	if (!SI.IsValid())
	{
		OnGISessionStarted.Execute(false);
		return;
	}

	StartSessionCompleteDelegateHandle =
		SI->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);

	if (!SI->StartSession(NAME_GameSession))
	{
		SI->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);

		OnGISessionStarted.Execute(false);
	}
}

void UMyGISubsystem::WithSessionInterface(TFunctionRef<void(IOnlineSessionPtr)> Func) const
{
	const IOnlineSessionPtr SI = Online::GetSessionInterface(GetWorld());
	if(SI.IsValid())
	{
		Func(SI);
	}
	else
	{
		UE_LOG
			( LogSubsystemCollection
			, Warning
			, TEXT("%s: session interface not valid")
			, *GetFullName()
			)
	}
}

