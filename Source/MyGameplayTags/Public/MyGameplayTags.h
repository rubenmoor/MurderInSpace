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

    // TODO: what is this tag for?
    FGameplayTag AccelerationMoveTowardsCircle;

    FGameplayTag InputBindingAbility;
    // translational acceleration ability
    FGameplayTag InputBindingAbilityAccelerate;
    // rotate to look at mouse ability
    FGameplayTag InputBindingAbilityLookAt;
    FGameplayTag InputBindingAbilityMoveTowardsCircle;
    FGameplayTag InputBindingAbilityEmbrace;
    FGameplayTag InputBindingAbilityKick;

    FGameplayTag Ability;
    FGameplayTag AbilityLookAt;

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
