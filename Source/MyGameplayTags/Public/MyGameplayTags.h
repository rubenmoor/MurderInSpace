#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Modules/ModuleManager.h"

enum class EInputAction : uint8;

struct MYGAMEPLAYTAGS_API FMyGameplayTags
{
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
    FGameplayTag AbilityRotate;
    FGameplayTag AbilityMoveTowardsCircle;
    
	FGameplayTag GiveInitiallyToMyPawn;
    
private:
    static FMyGameplayTags MyGameplayTags;
};

class GameplayTagsModule : public IModuleInterface
{
};
