#include "Menu/UW_Settings.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "Engine/UserInterfaceSettings.h"
#include "HUD/MyCommonButton.h"
#include "HUD/MyHUD.h"
#include "HUD/MyHUDMenu.h"
#include "Modes/MyLocalPlayer.h"
#include "Modes/MyGameUserSettings.h"

#define LOCTEXT_NAMESPACE "Settings"

void UUW_Settings::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    UE_LOG(LogMyGame, Warning, TEXT("UUW_Settings: NativeOnInitialized"))
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
    Resolutions.Add("custom"   , { 0  , 0    });
        
    for(const auto Res : Resolutions)
    {
        ComboBoxResolution->AddOption(Res.Key.ToString());
    }
    auto Res = GEngine->GetGameUserSettings()->GetScreenResolution();
    const FName* Name = Resolutions.FindKey(Res);
    if(Name)
    {
        ComboBoxResolution->SetSelectedOption(Name->ToString());
    }
    else
    {
        ComboBoxResolution->SetSelectedOption("custom");
    }
    
    BtnBack->OnClicked().AddLambda([this] ()
    {
        GoBack();
    });

    BtnApply->OnClicked().AddLambda([this] ()
    {
        GEngine->GetGameUserSettings()->ApplyResolutionSettings(false);
        GoBack();
    });
    CheckFullscreen->SetCheckedState
        ( GEngine->GetGameUserSettings()->GetFullscreenMode() == EWindowMode::Fullscreen
            ? ECheckBoxState::Checked
            : ECheckBoxState::Unchecked
        );
    SliderDPIScale->SetValue(GetMutableDefault<UUserInterfaceSettings>()->ApplicationScale);

    //ComboResolution->OnGenerateItemWidget.BindDynamic(this, &UUW_Settings::HandleComboResolutionItemGenerate);
    //ComboResolution->OnGenerateContentWidget.BindDynamic(this, &UUW_Settings::HandleComboResolutionGenerate);
    
    ComboBoxResolution->OnSelectionChanged.AddUniqueDynamic(this, &UUW_Settings::HandleResolutionSelect);
    SliderDPIScale->OnValueChanged.AddUniqueDynamic(this, &UUW_Settings::HandleDPIScaleValue);
    CheckFullscreen->OnCheckStateChanged.AddUniqueDynamic(this, &UUW_Settings::HandleCheckFullscreen);

    BtnApply->SetIsEnabled(GEngine->GetGameUserSettings()->IsDirty());
}

void UUW_Settings::NativeConstruct()
{
    Super::NativeConstruct();
}

void UUW_Settings::HandleResolutionSelect(FString Item, ESelectInfo::Type)
{
    auto* Settings = GEngine->GetGameUserSettings();
    Settings->SetScreenResolution(Resolutions[FName(*Item)]);
    BtnApply->SetIsEnabled(Settings->IsDirty());
}

void UUW_Settings::HandleDPIScaleValue(float Value)
{
    GetPlayerContext().GetHUD<AMyHUDBase>()->ScaleBorderWidget(Value);
    TextDPIScale->SetText(FText::AsNumber(Value, GetPlayerContext().GetHUD<AMyHUDBase>()->GetFODPIScale().Get()));
    GetMutableDefault<UUserInterfaceSettings>()->ApplicationScale = Value;
}

UWidget* UUW_Settings::HandleComboResolutionGenerate(FName Item)
{
    FText Text;
    if(Item == "custom")
    {
        auto Res = GEngine->GetGameUserSettings()->GetScreenResolution();
        Text = FText::Format(LOCTEXT("custom resolution", "{0}x{1}"), Res.X, Res.Y);
    }
    else
    {
        Text = FText::FromName(Item);
    }
    auto* TextItem = WidgetTree->ConstructWidget<UCommonTextBlock>();
    TextItem->SetStyle(TextStyle);
    TextItem->SetText(Text);
    return TextItem;
}

UWidget* UUW_Settings::HandleComboResolutionItemGenerate(FName Item)
{
    auto* TextItem = WidgetTree->ConstructWidget<UCommonTextBlock>();
    TextItem->SetStyle(TextStyle);
    TextItem->SetText(FText::FromName(Item));
    return TextItem;
}

void UUW_Settings::HandleCheckFullscreen(bool bChecked)
{
    GEngine->GameUserSettings->SetFullscreenMode(bChecked ? EWindowMode::Fullscreen : EWindowMode::Windowed);
    BtnApply->SetIsEnabled(GEngine->GameUserSettings->IsDirty());
}

void UUW_Settings::GoBack()
{
    if(Cast<UMyLocalPlayer>(GetPlayerContext().GetLocalPlayer())->GetIsInMainMenu())
    {
        GetPlayerContext().GetHUD<AMyHUDMenu>()->MenuMainShow();
    }
    else // in-game menu
    {
        GetPlayerContext().GetHUD<AMyHUD>()->InGameMenuShow();
    }
}

#undef LOCTEXT_NAMESPACE
