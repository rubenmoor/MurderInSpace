#pragma once

#include "CoreMinimal.h"
#include "MyGameplayAbility.h"

#include "GA_Accelerate.generated.h"

class UGE_AccelerateFire;
class UGE_AcceleratePosition;
class UInputAction;

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UGA_Accelerate : public UMyGameplayAbility
{
	GENERATED_BODY()

    UGA_Accelerate();

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<UGE_AcceleratePosition> GE_AcceleratePosition;
    TSubclassOf<UGE_AccelerateFire> GE_AccelerateFire;
    
    virtual FAbilityCoroutine ExecuteAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
