// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/MyInputTags.h"

#include "GameplayTagsManager.h"
#include "Modes/MyState.h"

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
