#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Modules/ModuleManager.h"

enum class EInputAction : uint8;

struct MYGAMEPLAYTAGS_API FMyGameplayTags
{
    FMyGameplayTags();
    static FMyGameplayTags& Get() { return MyGameplayTags; }
    
    const FGameplayTag& GetInputActionTag(EInputAction InputAction) const;

    FGameplayTag Acceleration;
    FGameplayTag AccelerationTranslational;
    FGameplayTag AccelerationRotational;
    FGameplayTag AccelerationRotationalDuration;
    
    FGameplayTag Ability;
    // translational acceleration ability
    FGameplayTag AbilityAccelerate;
    // rotate to look at mouse ability
    FGameplayTag AbilityRotate;
    FGameplayTag AbilityMoveTowardsCircle;

    // deprecated
    FGameplayTag HasTorque;
    FGameplayTag HasTorqueCCW;
    FGameplayTag HasTorqueCW;
    
    // initial values for gameplay attributes
    FGameplayTag TagInitialHealth;
private:
    TArray<FGameplayTag> InputActionTags;
    
    static FMyGameplayTags MyGameplayTags;
};

class GameplayTagsModule : public IModuleInterface
{
};
