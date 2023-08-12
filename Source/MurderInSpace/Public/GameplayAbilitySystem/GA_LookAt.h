#pragma once

#include "CoreMinimal.h"
#include "MyGameplayAbility.h"
#include "UE5CoroGAS/UE5CoroGameplayAbility.h"

#include "GA_LookAt.generated.h"

class UGE_TorqueCW;
class UGE_TorqueCCW;

using namespace UE5Coro;
using namespace UE5Coro::GAS;

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UGA_LookAt : public UMyGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_LookAt();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGE_TorqueCCW> GE_TorqueCCW;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGE_TorqueCW> GE_TorqueCW;
	
	virtual FAbilityCoroutine ExecuteAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	inline static FActiveGameplayEffectHandle TorqueHandle = INDEX_NONE;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float TransitionTime = 0.2;

	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;
};
