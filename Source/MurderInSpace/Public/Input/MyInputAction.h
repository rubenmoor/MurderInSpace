#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputAction.h"
#include "MyInputAction.generated.h"

/**
 * add tag container to input actions
 */
UCLASS()
class MURDERINSPACE_API UMyInputAction : public UInputAction
{
    GENERATED_BODY()
    
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FGameplayTagContainer Tags;
};
