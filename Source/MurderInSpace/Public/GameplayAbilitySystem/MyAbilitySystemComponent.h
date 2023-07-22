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
    
    /*
     * https://github.com/tranek/GASDocumentation/tree/19877c6cd777dc059ee1aa6e094eae1f3f2a4cab#483-local-gameplay-cues
     */
    UFUNCTION(BlueprintCallable, Category = "GameplayCue", Meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"))
    void ExecuteGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);

    UFUNCTION(BlueprintCallable, Category = "GameplayCue", Meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"))
    void AddGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);

    UFUNCTION(BlueprintCallable, Category = "GameplayCue", Meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"))
    void RemoveGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);

    TArray<FGameplayAbilitySpec> GetActiveAbilities(const FGameplayTagContainer* WithTags=nullptr, const FGameplayTagContainer* WithoutTags=nullptr, UGameplayAbility* Ignore=nullptr);
};
