#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MyGameMode.generated.h"

/**
 *  the `game mode` is server-side data only
 */
UCLASS()
class MURDERINSPACE_API AMyGameMode : public AGameMode
{
	GENERATED_BODY()

protected:
	// event handlers
	// prelogin
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
};
