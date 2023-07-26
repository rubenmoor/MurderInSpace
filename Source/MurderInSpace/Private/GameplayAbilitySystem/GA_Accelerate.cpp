#include "GameplayAbilitySystem/GA_Accelerate.h"

#include "Abilities/Tasks/AbilityTask_NetworkSyncPoint.h"
#include "MyGameplayTags.h"
#include "Modes/MyPlayerController.h"
#include "UE5Coro/LatentAwaiters.h"

UGA_Accelerate::UGA_Accelerate()
{
    const auto Tag = FMyGameplayTags::Get();
    InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
    AbilityTags.AddTag(Tag.AbilityAccelerate);
    ActivationOwnedTags.AddTag(Tag.AccelerationTranslational);
    // TODO
    //ActivationBlockedTags =
}

FAbilityCoroutine UGA_Accelerate::ExecuteAbility(FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if(!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        co_await Latent::Cancel();
    }
    auto* Orbit = Cast<IHasOrbit>(ActorInfo->OwnerActor)->GetOrbit();
    Orbit->UpdateVisibility(true);
    co_await Latent::Until([this] { return bReleased; });
    Orbit->UpdateVisibility(false);
}
