// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "GC_ShowOrbit.generated.h"

/**
 * target orbit to make visible
 */
UCLASS()
class MURDERINSPACE_API UGC_OrbitShow : public UGameplayCueNotify_Static
{
    GENERATED_BODY()

    UGC_OrbitShow();

    virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
    virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
};
