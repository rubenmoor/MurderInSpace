#include "Animation/AstronautAnimInstance.h"

#include "Actors/MyPawn_Humanoid.h"
#include "Logging/StructuredLog.h"
#include "Modes/MyGameInstance.h"

#if WITH_EDITOR
EDataValidationResult UAstronautAnimInstance::IsDataValid(TArray<FText>& ValidationErrors)
{
    Super::IsDataValid(ValidationErrors);
    GameplayTagPropertyMap.IsDataValid(this, ValidationErrors);
    return ValidationErrors.IsEmpty() ? EDataValidationResult::Valid : EDataValidationResult::Invalid;
}
#endif

void UAstronautAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    if(auto* Actor = GetOwningActor(); Actor->Implements<UAbilitySystemInterface>())
    {
        GameplayTagPropertyMap.Initialize(this, Cast<IAbilitySystemInterface>(Actor)->GetAbilitySystemComponent());
    }
    else
    {
        UE_LOGFMT(LogMyGame, Warning, "AnimInstance: NativeInitializeAnimation: no owning actor");
    }
}
