#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "MyTaggedInputAction.generated.h"

class UInputAction;
/**
 * FTaggedInputAction
 *
 *	Struct used to map an input action to a gameplay input tag.
 */
USTRUCT(BlueprintType)
struct MURDERINSPACE_API FTaggedInputAction
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
UCLASS()
class MURDERINSPACE_API UTaggedInputActionData : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "Data"))
    TMap<FGameplayTag, UInputAction*> Data;
};
