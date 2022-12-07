// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MyInputTags.generated.h"

class UInputAction;
/**
 * FTaggedInputAction
 *
 *	Struct used to map an input action to a gameplay input tag.
 */
USTRUCT(BlueprintType)
struct FTaggedInputAction
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly)
    const UInputAction* InputAction = nullptr;

    UPROPERTY(EditDefaultsOnly, Meta = (Categories = "InputTag"))
    FGameplayTag InputTag;
};

/**
 * 
 */
struct FInputTag
{
    void InitializeNativeTags();
    
    //Input Tags
    FGameplayTag ToggleIngameMenu;
    FGameplayTag ShowMyTrajectory;
    FGameplayTag HideMyTrajectory;
    FGameplayTag ShowAllTrajectories;
    FGameplayTag HideAllTrajectories;
    FGameplayTag ToggleMyTrajectory;
};

/**
 *
 */
UCLASS()
class MURDERINSPACE_API UMyInputActionsData : public UDataAsset
{
    GENERATED_BODY()

public:
    // Returns the first Input Action associated with a given tag.
    const UInputAction* FindInputActionForTag(const FGameplayTag& InputTag) const;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
    TArray<FTaggedInputAction> TaggedInputActions;
};