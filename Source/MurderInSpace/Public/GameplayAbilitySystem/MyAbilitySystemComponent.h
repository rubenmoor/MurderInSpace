#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "MyAbilitySystemComponent.generated.h"

class UMyEnhancedInputComponent;
struct FInputActionInstance;
class AMyPawn;

/**
 * 
 */
UCLASS(Blueprintable, ClassGroup=(Custom))
class MURDERINSPACE_API UMyAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

    UMyAbilitySystemComponent();
public:
    static UMyAbilitySystemComponent* Get(AMyPawn* InPawn);
    static UMyAbilitySystemComponent* Get(const FGameplayAbilityActorInfo* ActorInfo);

    UFUNCTION(BlueprintCallable)
    FGameplayTag FindTag(FGameplayTag InTag);
    
    TArray<FGameplayAbilitySpec> GetActiveAbilities(const FGameplayTagContainer* WithTags=nullptr, const FGameplayTagContainer* WithoutTags=nullptr, UGameplayAbility* Ignore=nullptr);

    DECLARE_DELEGATE(FDelegateStateFullyBlended)
    FDelegateStateFullyBlended OnStateFullyBlended;
};
