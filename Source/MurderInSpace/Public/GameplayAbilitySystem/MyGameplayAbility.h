#pragma once

#include "CoreMinimal.h"
#include "Logging/StructuredLog.h"
#include "Modes/MyGameInstance.h"
#include "UE5CoroGAS/UE5CoroGameplayAbility.h"

#include "MyGameplayAbility.generated.h"

class AMyCharacter;

using namespace UE5Coro::GAS;
using namespace UE5Coro;

struct FStopwatch
{
    void Start()
    {
        StartTime = FPlatformTime::Cycles64();
        bStarted = true;
    }
    double Stop()
    {
        if(!bStarted)
        {
            UE_LOGFMT(LogMyGame, Error, "FStopWatch::Stop() without running Start()");
            return 0.;
        }
        const uint64 EndTime = FPlatformTime::Cycles64();
        bStarted = false;
        return FPlatformTime::ToSeconds64(EndTime - StartTime);
    }
    void StopLog(FString StrInfo = "Time: ")
    {
        UE_LOGFMT(LogMyGame, Display, "{INFO}{DIFF}", StrInfo, FPlatformTime::PrettyTime(Stop()));
    }
private:
    bool bStarted = false;
    uint64 StartTime;
};

/**
 *  My base class for any ability that uses keys
 */
UCLASS(Blueprintable)
class MURDERINSPACE_API UMyGameplayAbility : public UUE5CoroGameplayAbility
{
	GENERATED_BODY()

public:
    UMyGameplayAbility();

    UFUNCTION(Server, Reliable)
    void ServerRPC_SetReleased();

    static void LocallyControlledDo(const FGameplayAbilityActorInfo* ActorInfo, std::function<void(const FLocalPlayerContext&)> Func);
protected:
    virtual FAbilityCoroutine ExecuteAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = GameplayEffects)
    bool RemoveActiveGameplayEffect(FActiveGameplayEffectHandle Handle, const FGameplayAbilityActorInfo& ActorInfo, FGameplayAbilityActivationInfo ActivationInfo, int32 StacksToRemove = -1);

    Private::FLatentAwaiter UntilReleased();

    bool bReleased = false;

    virtual FOnGameplayAbilityEnded* TurnBlocked(FGameplayAbilitySpecHandle InHandle, const FGameplayAbilityActorInfo* ActorInfo);

    FStopwatch Stopwatch;

    FDelegateHandle OnGameplayAbilityEndedHandle;
};
