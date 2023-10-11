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

    FGameplayTag Ability;
    FGameplayTag AbilityAccelerate;
    FGameplayTag AbilityMoveTowardsCircle;
    FGameplayTag AbilityEmbrace;
    FGameplayTag AbilityKick;
    FGameplayTag AbilityLookAt;
    FGameplayTag AbilityRecover;
    
    FGameplayTag BlockingTurn;

    // gameplay cues

    FGameplayTag GameplayCue;
    
    FGameplayTag CuePose;
    FGameplayTag CuePoseAccelerate;
    FGameplayTag CuePoseTorque;
    FGameplayTag CuePoseTorqueCCW;
    FGameplayTag CuePoseTorqueCW;
    
    FGameplayTag CuePoseEmbrace;
    FGameplayTag CuePoseEmbracePrepare;
    FGameplayTag CuePoseEmbraceExecute;
    
    FGameplayTag CuePoseKickPosition;
    FGameplayTag CuePoseKickExecute;
    
    FGameplayTag CueShowThrusters;
    FGameplayTag CueThrustersFire;
    
    FGameplayTag GameplayCueLocal;

    // tag input actions that are not bound to any gameplay ability
    FGameplayTag CustomInputBinding;
    FGameplayTag CustomInputBindingZoom;
    FGameplayTag CustomInputBindingSelect;
    FGameplayTag CustomInputBindingAllOrbitsShowHide;
    FGameplayTag CustomInputBindingMyOrbitShowHide;
    FGameplayTag CustomInputBindingIngameMenuToggle;

private:
    static const FMyGameplayTags Singleton;
};

class GameplayTagsModule : public IModuleInterface
{
};
