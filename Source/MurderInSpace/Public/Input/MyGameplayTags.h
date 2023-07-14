#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MyGameplayTags.generated.h"

enum class EInputAction : uint8;

struct MURDERINSPACE_API FMyGameplayTags
{
    FMyGameplayTags();
    static const FMyGameplayTags& Get() { return MyGameplayTags; }
    const FGameplayTag& GetInputActionTag(EInputAction InputAction) const;
private:
    TArray<FGameplayTag> InputActionTags;
    static FMyGameplayTags MyGameplayTags;
};

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