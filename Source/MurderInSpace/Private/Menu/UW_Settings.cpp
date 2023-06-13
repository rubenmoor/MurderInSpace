#include "Menu/UW_Settings.h"

#include "AnalogSlider.h"
#include "Components/ComboBoxKey.h"
#include "Engine/UserInterfaceSettings.h"
#include "HUD/MyCommonButton.h"
#include "HUD/MyHUD.h"
#include "HUD/MyHUDMenu.h"
#include "Modes/MyLocalPlayer.h"
#include "Modes/MyGameUserSettings.h"

void UUW_Settings::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    BtnBack->OnClicked().AddLambda([this] ()
    {
        if(Cast<UMyLocalPlayer>(GetPlayerContext().GetLocalPlayer())->GetIsInMainMenu())
        {
            GetPlayerContext().GetHUD<AMyHUDMenu>()->MenuMainShow();
        }
        else // in-game menu
        {
            GetPlayerContext().GetHUD<AMyHUD>()->InGameMenuShow();
        }
    });

    BtnApply->OnClicked().AddLambda([] ()
    {
        GEngine->GetGameUserSettings()->ApplyResolutionSettings(false);
    });

    ComboResolution->OnGenerateItemWidget.BindDynamic(this, &UUW_Settings::HandleComboResolutionGenerate);
    ComboResolution->OnSelectionChanged.AddUniqueDynamic(this, &UUW_Settings::HandleResolutionSelect);
    SliderDPIScale->OnAnalogCapture.AddUniqueDynamic(this, &UUW_Settings::HandleDPIScaleValue);

    if(!ComboResolution->OnGenerateContentWidget.IsBound())
    {
        UE_LOG(LogMyGame, Warning, TEXT("ComboResolution->OnGenerateContentWidget: not bound"))
    }
    if(!ComboResolution->OnSelectionChanged.IsBound())
    {
        UE_LOG(LogMyGame, Warning, TEXT("ComboResolution->OnSelectionChanged: not bound"))
    }
    if(!SliderDPIScale->OnAnalogCapture.IsBound())
    {
        UE_LOG(LogMyGame, Warning, TEXT("SliderDPIScale->OnAnalogCapture: not bound"))
    }
}

void UUW_Settings::NativeConstruct()
{
    Super::NativeConstruct();
}

void UUW_Settings::NativePreConstruct()
{
    Super::NativePreConstruct();
}

void UUW_Settings::HandleResolutionSelect(FName Item, ESelectInfo::Type)
{
    auto* Settings = GEngine->GetGameUserSettings();
    Settings->SetScreenResolution(Resolutions[Item]);
    BtnApply->SetIsEnabled(Settings->IsDirty());
}

void UUW_Settings::HandleDPIScaleValue(float Value)
{
    float New = 0.5 + Value;
    TextDPIScale->SetText(FText::AsNumber(New, GetPlayerContext().GetHUD<AMyHUDBase>()->GetFODPIScale().Get()));
    GetMutableDefault<UUserInterfaceSettings>()->ApplicationScale = New;
    UE_LOG(LogMyGame, Display, TEXT("DPI scale, new value: %.1f"), New);
}

UWidget* UUW_Settings::HandleComboResolutionGenerate(FName Item)
{
    Resolutions.Add("1280x720" , {1280, 720  });
    Resolutions.Add("1024x768" , {1024, 768  });
    Resolutions.Add("1360x768" , {1360, 768  });
    Resolutions.Add("1365x768" , {1365, 768  });
    Resolutions.Add("1280x800" , {1280, 800  });
    Resolutions.Add("1440x900" , {1440, 900  });
    Resolutions.Add("1600x900" , {1600, 900  });
    Resolutions.Add("1280x1024", {1280, 1024 });
    Resolutions.Add("1680x1050", {1680, 1050 });
    Resolutions.Add("1920x1080", {1920, 1080 });
    Resolutions.Add("2560x1080", {2560, 1080 });
    Resolutions.Add("1920x1200", {1920, 1200 });
    Resolutions.Add("2560x1440", {2560, 1440 });
    Resolutions.Add("3440x1440", {3440, 1440 });
    Resolutions.Add("3840x2160", {3840, 2160 });
    for(const auto Res : Resolutions)
    {
        ComboResolution->AddOption(Res.Key);
    }
    return ComboResolution;
}

