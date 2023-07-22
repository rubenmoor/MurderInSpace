#pragma once

#include <functional>

#include "CoreMinimal.h"
#include "UE5CoroGAS/UE5CoroGameplayAbility.h"

#include "MyGameplayAbility.generated.h"

struct FInputActionInstance;
class AMyPlayerController;
class UMyAbilitySystemComponent;

/**
 *  My base class for any ability that uses keys
 */
UCLASS(Blueprintable)
class MURDERINSPACE_API UMyGameplayAbility : public UUE5CoroGameplayAbility
{
	GENERATED_BODY()

public:
    DECLARE_DELEGATE(FOnReleaseDelegate)

    FOnReleaseDelegate OnReleaseDelegate;

protected:
    void BindOnRelease(std::function<void()> Callback);
};
