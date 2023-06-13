#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_Settings.generated.h"

class UMyCommonButton;
class UCommonTextBlock;
class UAnalogSlider;
class USlider;
class UComboBoxKey;

/**
 * 
 */
UCLASS()
class UUW_Settings : public UUserWidget
{
    GENERATED_BODY()

protected:
    // events
    virtual void NativePreConstruct() override;
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;

    // widgets
    
    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UComboBoxKey> ComboResolution;

    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UAnalogSlider> SliderDPIScale;

    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UCommonTextBlock> TextDPIScale;
    
    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UMyCommonButton> BtnApply;
    
    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UMyCommonButton> BtnBack;

private:
    TMap<FName, FIntPoint> Resolutions;

    // event handlers

    UFUNCTION()
    void HandleResolutionSelect(FName Item, ESelectInfo::Type _);
    
    UFUNCTION()
    void HandleDPIScaleValue(float Value);

    UFUNCTION(BlueprintCallable)
    UWidget* HandleComboResolutionGenerate(FName Item);
};

