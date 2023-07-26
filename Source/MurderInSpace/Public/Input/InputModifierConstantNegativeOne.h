#pragma once

#include "CoreMinimal.h"
#include "InputModifiers.h"
#include "InputModifierConstantNegativeOne.generated.h"

/**
 * 
 */
UCLASS(NotBlueprintable, meta=(DisplayName="Constant Negative One"))
class MURDERINSPACE_API UInputModifierConstantNegativeOne : public UInputModifier
{
    GENERATED_BODY()

protected:
    virtual FInputActionValue ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime) override;
};
