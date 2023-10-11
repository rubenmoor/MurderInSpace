#include "Modes/MyGameMode.h"

#include "Spacebodies/MyPlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"
#include "Modes/MyGameInstance.h"
#include "Modes/MyPlayerController.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

// void AMyGameMode::PostLogin(APlayerController* NewPlayer)
// {
// 	Super::PostLogin(NewPlayer);
// 	
// 	// doesn't work
// 	// at PostLogin there is no pawn yet
// 	if(!IsValid(NewPlayer->GetPawn()))
// 	{
// 		auto* GI = GetGameInstance();
// 		UMySessionManager::Get(GI)->LeaveSession();
// 	}
// }

AActor* AMyGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
#if WITH_EDITOR
	if(GetWorld()->WorldType == EWorldType::PIE && GEditor->IsSimulateInEditorInProgress())
	{
		return nullptr;
	}
#endif
	
	TArray<AActor*> Starts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMyPlayerStart::StaticClass(), Starts);
	// TODO UKismetArrayLibrary::Array_Shuffle()
	const auto Unoccupied = Starts.FilterByPredicate([] (AActor* Start) {
		return !Cast<AMyPlayerStart>(Start)->bIsOccupied;
	});
	const auto Enabled = Unoccupied.FilterByPredicate([] (AActor* Start)
	{
		return Cast<AMyPlayerStart>(Start)->bIsEnabled;
	});
	const auto Disabled = Unoccupied.FilterByPredicate([] (AActor* Start)
	{
		return !Cast<AMyPlayerStart>(Start)->bIsEnabled;
	});
	if(Unoccupied.IsEmpty())
	{
		UE_LOGFMT
			( LogNet
			, Error
			, "{0}: Many players, few Player Starts, you can't join."
			, GetFName()
			);
		Cast<AMyPlayerController>(Player)->ClientRPC_LeaveSession();
		return Starts.IsEmpty() ? nullptr : Starts[0];
	}

	AActor* StartActor;
	if(!Enabled.IsEmpty())
	{
		StartActor = Enabled[0];
	}
	else
	{
		StartActor = Disabled[0];
	}
	auto PlayerStart = Cast<AMyPlayerStart>(StartActor);
	PlayerStart->bIsOccupied = true;
	
	UE_LOGFMT
		( LogMyGame
		, Display
		, "Numplayers: {0}, {1} gets Start {2}"
		, NumPlayers
		, Player->GetFName()
		, PlayerStart->GetFName()
		);
	Cast<AMyPlayerController>(Player)->MyPlayerStart = PlayerStart;
	return StartActor;
}
