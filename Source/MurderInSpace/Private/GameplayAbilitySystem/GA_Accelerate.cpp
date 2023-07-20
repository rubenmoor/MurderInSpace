#include "GameplayAbilitySystem/GA_Accelerate.h"

#include "Abilities/Tasks/AbilityTask_NetworkSyncPoint.h"
#include "GameplayAbilitySystem/MyAbilitySystemComponent.h"
#include "GameplayAbilitySystem/MyGameplayTags.h"
#include "Orbit/Orbit.h"

UGA_Accelerate::UGA_Accelerate()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
    // TODO
    //ActivationBlockedTags =
}

void UGA_Accelerate::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                     const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    const auto TagIsAccelerating = FMyGameplayTags::Get().IsAccelerating;
    UMyAbilitySystemComponent::Get(ActorInfo)->AddGameplayTag(TagIsAccelerating);
    auto* Orbit = Cast<IHasOrbit>(ActorInfo->OwnerActor)->GetOrbit();
    Orbit->bIsChanging = true;
    BindOnRelease([=]
    {
        // TODO: requires instanced ability
        //UAbilityTask_NetworkSyncPoint::WaitNetSync(this, EAbilityTaskNetSyncType::OnlyServerWait);
        UMyAbilitySystemComponent::Get(ActorInfo)->RemoveGameplayTag(TagIsAccelerating);
        Orbit->bIsChanging = false;
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
    });
}


