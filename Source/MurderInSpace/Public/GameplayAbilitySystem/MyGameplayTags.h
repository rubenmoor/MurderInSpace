#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

enum class EInputAction : uint8;

struct MURDERINSPACE_API FMyGameplayTags
{
    FMyGameplayTags();
    static const FMyGameplayTags& Get() { return MyGameplayTags; }
    const FGameplayTag& GetInputActionTag(EInputAction InputAction) const;
    
    // initial values for gameplay attributes
    FGameplayTag TagInitialHealth;
private:
    TArray<FGameplayTag> InputActionTags;
    
    static FMyGameplayTags MyGameplayTags;

};
