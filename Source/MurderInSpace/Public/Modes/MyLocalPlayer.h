#pragma once

#include "CoreMinimal.h"
#include "Engine/LocalPlayer.h"
#include "MyLocalPlayer.generated.h"

UENUM(BlueprintType)
enum class ECurrentLevel : uint8
{
	MainMenu UMETA(DisplayName="MainMenu"),
	SpaceFootball UMETA(DisplayName="Spacefootball")
};

UENUM(BlueprintType)
enum class EInGame : uint8
{
	  IngameUndefined UMETA(DisplayName="undefined")
	, IngamePlaying   UMETA(DisplayName="playing")
	, IngameJoining   UMETA(DisplayName="joining game")
	, IngameMenu      UMETA(DisplayName="in-game menu")
};

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UMyLocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	bool GetIsInMainMenu()
	{
		return CurrentLevel == ECurrentLevel::MainMenu;
	}

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ECurrentLevel CurrentLevel = ECurrentLevel::MainMenu;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool IsMultiplayer = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	EInGame InGame = EInGame::IngameUndefined;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool IsLoggedIn = false;
};
