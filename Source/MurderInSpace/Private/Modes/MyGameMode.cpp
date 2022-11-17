// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MyGameMode.h"

#include "Actors/MyPlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Modes/MyPlayerController.h"

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
		// TODO
		return nullptr;
	}
	const FVector Loc = Starts[NumPlayers]->GetActorLocation();
	UE_LOG
		( LogNet
		, Display
		, TEXT("%s: Player %d gets Start %d: (%.0f, %.0f, %.0f)")
		, *GetFullName()
		, NumPlayers
		, NumPlayers
		, Loc.X
		, Loc.Y
		, Loc.Z
		)
	return Starts[NumPlayers];
}
