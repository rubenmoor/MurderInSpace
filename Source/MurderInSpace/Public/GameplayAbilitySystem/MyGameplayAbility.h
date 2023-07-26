#pragma once

#include "CoreMinimal.h"
#include "UE5CoroGAS/UE5CoroGameplayAbility.h"

#include "MyGameplayAbility.generated.h"

using namespace UE5Coro::GAS;
using namespace UE5Coro;

/**
 *  My base class for any ability that uses keys
 */
UCLASS(Blueprintable)
class MURDERINSPACE_API UMyGameplayAbility : public UUE5CoroGameplayAbility
{
	GENERATED_BODY()

protected:
    bool bReleased = false;
    
public:
    void SetReleased() { bReleased = true; }

protected:
    virtual FAbilityCoroutine ExecuteAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
