#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "MyTaggedInputAction.generated.h"

class UInputAction;

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
