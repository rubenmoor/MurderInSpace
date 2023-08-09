#pragma once

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
    FGameplayTag AccelerationRotationalCCW;
    FGameplayTag AccelerationRotationalCW;

	// TODO: remove completely, probably
    FGameplayTag AccelerationMoveTowardsCircle;

    FGameplayTag InputBindingAbility;
    
    // translational acceleration ability
    FGameplayTag InputBindingAbilityAccelerate;
    
    FGameplayTag InputBindingAbilityMoveTowardsCircle;
    FGameplayTag InputBindingAbilityEmbrace;
    FGameplayTag InputBindingAbilityKick;

    // TODO: not currently used
    FGameplayTag Ability;
    FGameplayTag AbilityAccelerate;
    FGameplayTag AbilityLookAt;
    FGameplayTag AbilityEmbrace;
    FGameplayTag AbilityKick;

    // gameplay cues

    FGameplayTag GameplayCue;
    
    FGameplayTag CuePose;
    FGameplayTag CuePoseAccelerate;
    FGameplayTag CuePoseTurn;
    FGameplayTag CuePoseTurnCCW;
    FGameplayTag CuePoseTurnCW;
    FGameplayTag CuePoseEmbrace;
    FGameplayTag CuePoseKickPosition;
    FGameplayTag CuePoseKickExecute;
    
    FGameplayTag CueAccelerateShowThrusters;
    FGameplayTag CueAccelerateFire;
    
    FGameplayTag GameplayCueLocal;
    FGameplayTag LocalCueAccelerateFire;

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
