#pragma once

#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "Modes/MyPlayerController.h"
#include "InputTriggers.h"

#include "MyInputActionSet.generated.h"

class AMyPlayerController;
class UInputTrigger;

UCLASS()
class MURDERINSPACE_API UMyInputActionSet : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TArray<FKey> Keys;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSet<EInputTrigger> InputTriggers;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FGameplayTagContainer InputActionTags;

    // create and bind an input action for every trigger in `InputTriggers`
    UFUNCTION(BlueprintCallable)
    void BindActions(AMyPlayerController* InPlayercontroller, UInputMappingContext* IMC);

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input Action Settings")
	float ActuationThreshold = 0.5f;

    // Hold, HoldAndRelease, Tap, Pulse
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Action Settings")
	bool bAffectedByTimeDilation = false;

    // Hold, HoldAndRelease
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Action Settings")
	float HoldTimeThreshold = 0.2f;
    
    // Hold, HoldAndRelease
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Action Settings")
	bool bIsOneShot = true;

    // Tap
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Action Settings")
	float TapReleaseTimeThreshold = 0.2f;

protected:
    template<EInputTrigger InputTrigger>
    void BindAction(UInputMappingContext* IMC);

    UInputTrigger* GetTriggerEvent(EInputTrigger InInputTrigger);

    template<EInputTrigger InputTrigger>
    void HandleInput(const FInputActionInstance& InputActionInstance);

    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<AMyPlayerController> PC;
};

template <EInputTrigger InputTrigger>
void UMyInputActionSet::BindAction(UInputMappingContext* IMC)
{
    auto* InputAction = NewObject<UInputAction>(this);
    InputAction->Triggers.Add(GetTriggerEvent(InputTrigger));
    if(InputTriggers.Contains(InputTrigger))
    {
        PC->GetInputComponent()->BindAction(InputAction, ETriggerEvent::Triggered, this, &UMyInputActionSet::HandleInput<InputTrigger>);
    }

    for(const auto& Key : Keys)
    {
        IMC->MapKey(InputAction, Key);
    }
}

template <EInputTrigger InputTrigger>
void UMyInputActionSet::HandleInput(const FInputActionInstance& InputActionInstance)
{
    PC->RunInputAction(InputActionTags, InputTrigger, InputActionInstance);
}


