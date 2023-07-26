#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "MyInputActions.generated.h"

class UMyInputActionSet;

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UMyInputActions : public UDataAsset
{
    GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Action Assets")
	TMap<FName, UMyInputActionSet*> Map;

#if WITH_EDITOR
protected:
	UFUNCTION(CallInEditor)
	void RefreshMyInputActions();
#endif
	
};
