#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"

#include "GA_Torque.generated.h"

/**
 * base class for TorqueCCW and TorqueCW Blueprint Abilities
 * need to set the tags in the editor to work correctly
 */
UCLASS()
class MURDERINSPACE_API UGA_Torque : public UGameplayAbility
{
	GENERATED_BODY()

	UGA_Torque();

protected:
	// TODO: remove when moving away from `UGameplayAbility` as base class
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
