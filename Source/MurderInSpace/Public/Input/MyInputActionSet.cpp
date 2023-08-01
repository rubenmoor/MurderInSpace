#include "MyInputActionSet.h"
#include "InputTriggers.h"

void UMyInputActionSet::BindActions(AMyPlayerController* InPlayercontroller, UInputMappingContext* IMC)
{
    check(!InputActionTags.IsEmpty())
    
    PC = InPlayercontroller;
    
    using enum EInputTrigger;
    BindAction<Down>          (IMC);
    BindAction<Pressed>       (IMC);
    BindAction<Released>      (IMC);
    BindAction<Hold>          (IMC);
    BindAction<HoldAndRelease>(IMC);
    BindAction<Tap>           (IMC);
    BindAction<Pulse>         (IMC);
    BindAction<ChordAction>   (IMC);
}

/**
 * 
 * @param InInputTrigger 
 * @return create an InputTrigger UObject based on the enum
 */
// ReSharper disable once CppNotAllPathsReturnValue
UInputTrigger* UMyInputActionSet::GetTriggerEvent(EInputTrigger InInputTrigger)
{
    switch (InInputTrigger)
    {
    case EInputTrigger::Down:
        {
            const auto InputTrigger = NewObject<UInputTriggerDown>(this);
            InputTrigger->ActuationThreshold = ActuationThreshold;
            return InputTrigger;
        }
    case EInputTrigger::Pressed:
        {
            const auto InputTrigger = NewObject<UInputTriggerPressed>(this);
            InputTrigger->ActuationThreshold = ActuationThreshold;
            return InputTrigger;
        }
    case EInputTrigger::Released:
        {
            const auto InputTrigger = NewObject<UInputTriggerReleased>(this);
            InputTrigger->ActuationThreshold = ActuationThreshold;
            return InputTrigger;
        }
    case EInputTrigger::Hold:
        {
            const auto InputTrigger = NewObject<UInputTriggerHold>(this);
            InputTrigger->ActuationThreshold = ActuationThreshold;
            InputTrigger->bAffectedByTimeDilation = bAffectedByTimeDilation;
            InputTrigger->HoldTimeThreshold = HoldTimeThreshold;
            InputTrigger->bIsOneShot = bIsOneShot;
            return InputTrigger;
        }
    case EInputTrigger::HoldAndRelease:
        {
            const auto InputTrigger = NewObject<UInputTriggerHoldAndRelease>(this);
            InputTrigger->ActuationThreshold = ActuationThreshold;
            InputTrigger->bAffectedByTimeDilation = bAffectedByTimeDilation;
            InputTrigger->HoldTimeThreshold = HoldTimeThreshold;
            return InputTrigger;
        }
    case EInputTrigger::Tap:
        {
            const auto InputTrigger = NewObject<UInputTriggerTap>(this);
            InputTrigger->ActuationThreshold = ActuationThreshold;
            InputTrigger->TapReleaseTimeThreshold = TapReleaseTimeThreshold;
            return InputTrigger;
        }
    case EInputTrigger::Pulse:
        {
            const auto InputTrigger = NewObject<UInputTriggerPulse>(this);
            InputTrigger->ActuationThreshold = ActuationThreshold;
            InputTrigger->bAffectedByTimeDilation = bAffectedByTimeDilation;
            return InputTrigger;
        }
    case EInputTrigger::ChordAction:
        {
            const auto InputTrigger = NewObject<UInputTriggerChordAction>(this);
            InputTrigger->ActuationThreshold = ActuationThreshold;
            return InputTrigger;
        }
    }
}
