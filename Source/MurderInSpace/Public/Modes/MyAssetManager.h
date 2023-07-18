

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "MyAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UMyAssetManager : public UAssetManager
{
	GENERATED_BODY()

protected:
    virtual void StartInitialLoading() override;
};
