﻿#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMyGameplayTags, All, All);

enum class EInputAction : uint8;

struct MYGAMEPLAYTAGS_API FMyGameplayTags : FNoncopyable
{
    FMyGameplayTags();
    static const FMyGameplayTags& Get() { return Singleton; }

    FGameplayTag Acceleration;
    FGameplayTag AccelerationTranslational;
    FGameplayTag AccelerationRotational;
    FGameplayTag AccelerationRotationalDuration;
    FGameplayTag AccelerationRotationalCCW;
    FGameplayTag AccelerationRotationalCW;

    FGameplayTag Embrace;
    FGameplayTag KickPosition;
    FGameplayTag KickExecute;
    
	// TODO: remove completely, probably
    FGameplayTag AccelerationMoveTowardsCircle;

    FGameplayTag InputBindingAbility;
    // translational acceleration ability
    FGameplayTag InputBindingAbilityAccelerate;
    
    // rotate to look at mouse ability
    // this tag isn't in use. LookAt is triggered by mouse movement
    FGameplayTag InputBindingAbilityLookAt;
    
    FGameplayTag InputBindingAbilityMoveTowardsCircle;
    FGameplayTag InputBindingAbilityEmbrace;
    FGameplayTag InputBindingAbilityKick;

    FGameplayTag Ability;
    FGameplayTag AbilityAccelerate;
    FGameplayTag AbilityLookAt;
    FGameplayTag AbilityEmbrace;
    FGameplayTag AbilityKick;

    // gameplay cues

    FGameplayTag GameplayCue;
    FGameplayTag CueAccelerate;
    FGameplayTag GameplayCueLocal;

    // tags to mark input actions that are not bound to any gameplay ability or gameplay cue
    FGameplayTag InputBindingCustom;
    FGameplayTag InputBindingCustomZoom;
    FGameplayTag InputBindingCustomSelect;
    FGameplayTag InputBindingCustomAllOrbitsShowHide;
    FGameplayTag InputBindingCustomMyOrbitShowHide;
    FGameplayTag InputBindingCustomIngameMenuToggle;

private:
    static const FMyGameplayTags Singleton;
};

class GameplayTagsModule : public IModuleInterface
{
};
