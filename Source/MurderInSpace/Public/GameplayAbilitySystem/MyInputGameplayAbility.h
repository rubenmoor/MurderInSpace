#pragma once

#include <functional>

#include "CoreMinimal.h"
#include "InputAction.h"
#include "Abilities/GameplayAbility.h"

#include "MyInputGameplayAbility.generated.h"

struct FInputActionInstance;
class AMyPlayerController;
class UMyAbilitySystemComponent;

/**
 *  My base class for any ability that uses keys
 */
UCLASS()
class MURDERINSPACE_API UMyInputGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UInputAction> InputAction;

    DECLARE_DELEGATE(FOnReleaseDelegate)

    FOnReleaseDelegate OnReleaseDelegate;

    void BindOnRelease(std::function<void()> Callback);

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
