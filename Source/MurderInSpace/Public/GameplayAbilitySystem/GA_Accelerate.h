#pragma once

#include "CoreMinimal.h"
#include "MyGameplayAbility.h"
#include "GA_Accelerate.generated.h"

class UInputAction;
/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UGA_Accelerate : public UMyGameplayAbility
{
	GENERATED_BODY()

    UGA_Accelerate();

    virtual UE5Coro::GAS::FAbilityCoroutine ExecuteAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
