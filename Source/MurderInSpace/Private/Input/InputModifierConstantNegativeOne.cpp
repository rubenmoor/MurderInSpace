#include "Input/InputModifierConstantNegativeOne.h"

FInputActionValue UInputModifierConstantNegativeOne::ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput,
    FInputActionValue CurrentValue, float DeltaTime)
{
    return FVector(-1., 0., 0.);
}
