#pragma once

#include "CoreMinimal.h"
#include "CommonTextBlock.h"
#include "Blueprint/UserWidget.h"
#include "UW_Settings.generated.h"

class UCheckBox;
class UMyCommonButton;
class UCommonTextBlock;
class USlider;
class UComboBoxString;

/**
 * 
 */
UCLASS()
class UUW_Settings : public UUserWidget
{
    GENERATED_BODY()

protected:
    // events
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;

    // widgets
    
    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UComboBoxString> ComboBoxResolution;

    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UCheckBox> CheckFullscreen;
    
    UPROPERTY(meta=(BindWidget))
    TObjectPtr<USlider> SliderDPIScale;

    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UCommonTextBlock> TextDPIScale;
    
    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UMyCommonButton> BtnApply;
    
    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UMyCommonButton> BtnBack;

    UPROPERTY(EditAnywhere)
    TSubclassOf<UCommonTextStyle> TextStyle;

private:
    TMap<FName, FIntPoint> Resolutions;
    
    // event handlers

    UFUNCTION()
    void HandleResolutionSelect(FString Item, ESelectInfo::Type _);
    
    UFUNCTION()
    void HandleDPIScaleValue(float Value);

    UFUNCTION()
    UWidget* HandleComboResolutionGenerate(FName Item);

    UFUNCTION()
    UWidget* HandleComboResolutionItemGenerate(FName Item);

    UFUNCTION()
    void HandleCheckFullscreen(bool bChecked);

    void GoBack();
};

