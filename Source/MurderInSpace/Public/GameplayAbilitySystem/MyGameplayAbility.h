#pragma once

#include "CoreMinimal.h"
#include "SpaceBodies/MyCharacter.h"
#include "UE5CoroGAS/UE5CoroGameplayAbility.h"

#include "MyGameplayAbility.generated.h"

class AMyCharacter;

using namespace UE5Coro::GAS;
using namespace UE5Coro;

/**
 *  My base class for any ability that uses keys
 */
UCLASS(Blueprintable)
class MURDERINSPACE_API UMyGameplayAbility : public UUE5CoroGameplayAbility
{
	GENERATED_BODY()

public:
    UMyGameplayAbility();
    void SetReleased() { bReleased = true; }

    static void LocallyControlledDo(const FGameplayAbilityActorInfo* ActorInfo, std::function<void(AMyCharacter*)> Func);
protected:
    virtual FAbilityCoroutine ExecuteAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

    Private::FLatentAwaiter UntilReleased();

    bool bReleased = false;
};
