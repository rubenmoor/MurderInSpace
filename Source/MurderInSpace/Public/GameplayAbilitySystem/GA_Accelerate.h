

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySystem/MyGameplayAbilityBase.h"
#include "GA_Accelerate.generated.h"

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UGA_Accelerate : public UMyGameplayAbilityBase
{
	GENERATED_BODY()

    UGA_Accelerate();
    
protected:
    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<UGameplayEffect> GameplayEffect;
    
    FActiveGameplayEffectHandle GameplayEffectHandle;

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
    
    // event handlers

    virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
};
