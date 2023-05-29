#include "Modes/MyGameMode.h"

#include "Actors/MyPlayerStart.h"
#include "Kismet/GameplayStatics.h"
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
		UE_LOG
			( LogNet
			, Error
			, TEXT("%s: Many players, few Player Starts, you can't join.")
			, *GetFullName()
			)
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
	
	const FVector Loc = StartActor->GetActorLocation();
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
	Cast<AMyPlayerController>(Player)->MyPlayerStart = PlayerStart;
	return StartActor;
}
