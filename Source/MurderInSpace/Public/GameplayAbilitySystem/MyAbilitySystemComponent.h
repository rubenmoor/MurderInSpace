#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "MyAbilitySystemComponent.generated.h"

class AMyPawn;
/**
 * 
 */
UCLASS(Blueprintable, ClassGroup=(Custom))
class MURDERINSPACE_API UMyAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
    
public:
    static UMyAbilitySystemComponent* Get(AMyPawn* InPawn);
    
    UFUNCTION(BlueprintCallable)
    void ApplyGE_MoveTowardsCircle();
    
    UFUNCTION(BlueprintCallable)
    void RemoveGE_MoveTowardsCircle();

    /*
     * local gameplay cues TODO: should I use these for UI?
     * https://github.com/tranek/GASDocumentation/tree/19877c6cd777dc059ee1aa6e094eae1f3f2a4cab#483-local-gameplay-cues
     */
    UFUNCTION(BlueprintCallable, Category = "GameplayCue", Meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"))
    void ExecuteGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);

    UFUNCTION(BlueprintCallable, Category = "GameplayCue", Meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"))
    void AddGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);

    UFUNCTION(BlueprintCallable, Category = "GameplayCue", Meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"))
    void RemoveGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);
    
    // event handlers

    virtual void BeginPlay() override;

protected:
    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<UGameplayEffect> GE_MoveTowardsCircle;

    FActiveGameplayEffectHandle GE_MoveTowardsCircleHandle;
};
