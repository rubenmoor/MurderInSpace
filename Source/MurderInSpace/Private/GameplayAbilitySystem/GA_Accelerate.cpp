#include "GameplayAbilitySystem/GA_Accelerate.h"

#include "Abilities/Tasks/AbilityTask_NetworkSyncPoint.h"
#include "MyGameplayTags.h"
#include "Orbit/Orbit.h"

UGA_Accelerate::UGA_Accelerate()
{
    const auto Tag = FMyGameplayTags::Get();
    InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
    AbilityTags.AddTag(Tag.AbilityAccelerate);
    ActivationOwnedTags.AddTag(Tag.AccelerationTranslational);
    // TODO
    //ActivationBlockedTags =
}

UE5Coro::GAS::FAbilityCoroutine UGA_Accelerate::ExecuteAbility(FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    auto* Orbit = Cast<IHasOrbit>(ActorInfo->OwnerActor)->GetOrbit();
    Orbit->bIsChanging = true;
    BindOnRelease([=]
    {
        // TODO: requires instanced ability
        //UAbilityTask_NetworkSyncPoint::WaitNetSync(this, EAbilityTaskNetSyncType::OnlyServerWait);
        Orbit->bIsChanging = false;
    });
    co_return;
}