// 


#include "GC_ShowOrbit.h"

#include "MyGameplayTags.h"
#include "Orbit/Orbit.h"

UGC_OrbitShow::UGC_OrbitShow()
{
    const auto Tag = FMyGameplayTags::Get();
    //GameplayCueTag = Tag.CueOrbitShow;
}

bool UGC_OrbitShow::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
    const auto Orbit = Cast<AOrbit>(MyTarget);
    check(IsValid(Orbit))
    Orbit->UpdateVisibility(true);
    return true;
}

bool UGC_OrbitShow::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
    const auto Orbit = Cast<AOrbit>(MyTarget);
    check(IsValid(Orbit))
    Orbit->UpdateVisibility(false);
    return true;
}
