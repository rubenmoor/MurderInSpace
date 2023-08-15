#include "Animation/AstronautAnimInstance.h"

#include "GameplayAbilitySystem/MyAbilitySystemComponent.h"
#include "Spacebodies/MyPawn_Humanoid.h"
#include "Logging/StructuredLog.h"
#include "Modes/MyGameInstance.h"

void UAstronautAnimInstance::HandleStateFullyBlended()
{
    auto* ASC = UMyAbilitySystemComponent::Get(Cast<AMyPawn>(GetOwningActor()));
    ASC->OnAnimStateFullyBlended.ExecuteIfBound();
    ASC->bAnimStateFullyBlended = true;
}

void UAstronautAnimInstance::HandleStateLeft()
{
    UMyAbilitySystemComponent::Get(Cast<AMyPawn>(GetOwningActor()))->OnAnimStateLeft.ExecuteIfBound();
}

void UAstronautAnimInstance::HandleStateEntered()
{
    if(GetWorld()->WorldType == EWorldType::EditorPreview)
        return;

    UMyAbilitySystemComponent::Get(Cast<AMyPawn>(GetOwningActor()))->OnAnimStateEntered.ExecuteIfBound();
}

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
