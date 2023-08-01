#pragma once

#include "CoreMinimal.h"
#include "MyGameplayAbility.h"
#include "UE5CoroGAS/UE5CoroGameplayAbility.h"

#include "GA_LookAt.generated.h"

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
	TSubclassOf<UGameplayEffect> GE_TorqueCCW;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> GE_TorqueCW;
	
	virtual FAbilityCoroutine ExecuteAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
};
