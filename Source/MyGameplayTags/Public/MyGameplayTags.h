#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Modules/ModuleManager.h"

#include "MyGameplayTags.generated.h"

enum class EInputAction : uint8;

USTRUCT(BlueprintType)
struct MYGAMEPLAYTAGS_API FMyGameplayTags
{
    GENERATED_BODY()
    
    FMyGameplayTags();
    
    static FMyGameplayTags& Get() { return MyGameplayTags; }
    
    FGameplayTag Acceleration;
    FGameplayTag AccelerationTranslational;
    FGameplayTag AccelerationRotational;
    FGameplayTag AccelerationRotationalDuration;
    FGameplayTag AccelerationRotationalCCW;
    FGameplayTag AccelerationRotationalCW;

    FGameplayTag Ability;
    // translational acceleration ability
    FGameplayTag AbilityAccelerate;
    // rotate to look at mouse ability
    FGameplayTag AbilityLookAt;
    FGameplayTag AbilityMoveTowardsCircle;
    FGameplayTag AbilityEmbrace;
    FGameplayTag AbilityKick;

	FGameplayTag GiveInitiallyToMyPawn;

    FGameplayTag GameplayCue;

    // tags to mark input actions that are not bound to any gameplay ability or gameplay cue
    FGameplayTag InputBindingCustom;
    FGameplayTag InputBindingCustomZoom;
    FGameplayTag InputBindingCustomSelect;
    FGameplayTag InputBindingCustomAllOrbitsShowHide;
    FGameplayTag InputBindingCustomMyOrbitShowHide;
    FGameplayTag InputBindingCustomIngameMenuToggle;
private:
    static FMyGameplayTags MyGameplayTags;
};

class GameplayTagsModule : public IModuleInterface
{
};
