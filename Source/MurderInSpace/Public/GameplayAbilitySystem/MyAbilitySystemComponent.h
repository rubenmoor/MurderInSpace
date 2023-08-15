#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "UE5CoroGAS/UE5CoroGameplayAbility.h"

#include "MyAbilitySystemComponent.generated.h"

class UMyGameplayAbility;
class IAbilitySystemInterface;
class UMyEnhancedInputComponent;
struct FInputActionInstance;
class AMyPawn;

using namespace UE5Coro;

UENUM()
enum class EPoseCue : uint8
{
    Add, Remove
};

/**
 * 
 */
UCLASS(Blueprintable, ClassGroup=(Custom))
class MURDERINSPACE_API UMyAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

    UMyAbilitySystemComponent();
public:
    static UMyAbilitySystemComponent* Get(const IAbilitySystemInterface* InPawn);
    static UMyAbilitySystemComponent* Get(const FGameplayAbilityActorInfo* ActorInfo);

    UFUNCTION(BlueprintCallable)
    FGameplayTag FindTag(FGameplayTag InTag);
    
    TArray<FGameplayAbilitySpec> GetActiveAbilities
        ( const FGameplayTagContainer& WithAnyTag = FGameplayTagContainer()
        , const FGameplayTagContainer& WithoutTags = FGameplayTagContainer()
        , TArray<FGameplayAbilitySpecHandle> IgnoreList = {}
        );

    void SendGameplayEvent(FGameplayTag Tag, FGameplayEventData EventData);

    using UAbilitySystemComponent::CancelAbilitySpec;
    
    // only add the cue, if the ability system component doesn't have it already
    // return true if a cue was newly added
    UFUNCTION(BlueprintCallable)
    bool AddGameplayCueUnlessExists(FGameplayTag Cue);

    // only try to remove the cue, if the ability system component has it
    // return true if a cue was actually removed
    UFUNCTION(BlueprintCallable)
    bool RemoveGameplayCueIfExists(FGameplayTag Cue);

    UPROPERTY()
    FGameplayAbilitySpecHandle AbilityAwaitingTurn;
    
    DECLARE_DELEGATE(FOnAnimStateFullyBlended)
    FOnAnimStateFullyBlended OnAnimStateFullyBlended;

    DECLARE_DELEGATE(FOnAnimStateFullyBlended)
    FOnAnimStateFullyBlended OnAnimStateEntered;

    DECLARE_DELEGATE(FOnAnimStateFullyBlended)
    FOnAnimStateFullyBlended OnAnimStateLeft;

    bool bAnimStateFullyBlended = true;

    bool AddPoseCue(FGameplayTag PoseCue);
    bool RemovePoseCue(FGameplayTag PoseCue);

    bool HasPose(FGameplayTag PoseCue) const;

private:
    using UAbilitySystemComponent::AddGameplayCue;
    using UAbilitySystemComponent::RemoveGameplayCue;
};
