#pragma once

#include "CoreMinimal.h"
#include "MyInputGameplayAbility.h"
#include "GA_Accelerate.generated.h"

class UInputAction;
/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UGA_Accelerate : public UMyInputGameplayAbility
{
	GENERATED_BODY()

    UGA_Accelerate();

protected:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
