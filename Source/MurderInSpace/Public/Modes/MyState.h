#pragma once

#include <random>

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"

#include "MyState.generated.h"

class UMyGameInstance;
class ABlackhole;

DECLARE_LOG_CATEGORY_EXTERN(LogMyGame, All, All);

USTRUCT(BlueprintType)
struct FRnd
{
	GENERATED_BODY()

	std::default_random_engine RndGen;
	std::poisson_distribution<int> Poisson;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FRandomStream Stream;
};

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UMyState : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	static UMyState* Get() { return GEngine->GetEngineSubsystem<UMyState>(); }
	
    /**
     * @brief constant factor to construct tangents for spline points
     */
    static constexpr double SplineToCircle = 1.6568542494923806; // 4. * (sqrt(2) - 1.);

	// private members
};
