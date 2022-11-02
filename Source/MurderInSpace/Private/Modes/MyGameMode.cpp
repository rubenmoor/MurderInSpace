// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MyGameMode.h"

#include "Actors/MyPlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Modes/MyPlayerController.h"

void AMyGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId,
                           FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	UE_LOG
		( LogNet
		, Warning
		, TEXT("%s: PreLogin\n unique net id: %s")
		, *GetFullName()
		, *UniqueId.ToString()
		)
}

AActor* AMyGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<AActor*> Starts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMyPlayerStart::StaticClass(), Starts);
	if(NumPlayers >= Starts.Num())
	{
		UE_LOG
			( LogNet
			, Error
			, TEXT("%s: Many players, few Player Starts, you can't join.")
			, *GetFullName()
			)
		Cast<AMyPlayerController>(Player)->ClientRPC_LeaveSession();
		// shouldn't matter, feels better than `nullptr`
		return Starts[0];
	}
	UE_LOG
		( LogNet
		, Display
		, TEXT("%s: Player %d gets Start %d")
		, *GetFullName()
		, NumPlayers
		, NumPlayers
		)
	return Starts[NumPlayers];
}
