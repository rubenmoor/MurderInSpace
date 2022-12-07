// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/MyInputTags.h"

#include "GameplayTagsManager.h"
#include "Modes/MyState.h"

void FInputTag::InitializeNativeTags()
{
    UGameplayTagsManager& GameplayTagsManager = UGameplayTagsManager::Get();
    
    ToggleIngameMenu    = GameplayTagsManager.AddNativeGameplayTag(FName(TEXT("InputTag.ToggleIngameMenu")));
    ShowMyTrajectory    = GameplayTagsManager.AddNativeGameplayTag(FName(TEXT("InputTag.ShowMyTrajectory")));
    HideMyTrajectory    = GameplayTagsManager.AddNativeGameplayTag(FName(TEXT("InputTag.HideMyTrajectory")));
    ShowAllTrajectories = GameplayTagsManager.AddNativeGameplayTag(FName(TEXT("InputTag.ShowAllTrajectories")));
    HideAllTrajectories = GameplayTagsManager.AddNativeGameplayTag(FName(TEXT("InputTag.HideAllTrajectories")));
    ToggleMyTrajectory  = GameplayTagsManager.AddNativeGameplayTag(FName(TEXT("InputTag.ToggleMyTrajectory")));
    
    GameplayTagsManager.DoneAddingNativeTags();
}

const UInputAction* UMyInputActionsData::FindInputActionForTag(const FGameplayTag& InputTag) const
{
    for (const FTaggedInputAction& TaggedInputAction : TaggedInputActions)
    {
        if (TaggedInputAction.InputTag == InputTag)
        {
            return TaggedInputAction.InputAction;
        }
    }
    UE_LOG(LogMyGame, Error, TEXT("%s: no tagged input action with tag %s"), *GetFullName(), *InputTag.ToString())
    return nullptr;
}