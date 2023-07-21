#include "GameplayAbilitySystem/GA_LookAt.h"

#include "GameplayAbilitySystem/MyAttributes.h"
#include "MyGameplayTags.h"
#include "UE5Coro/LatentAwaiters.h"

using namespace UE5Coro;
using namespace UE5Coro::GAS;

UGA_LookAt::UGA_LookAt()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
}

FAbilityCoroutine UGA_LookAt::ExecuteAbility(FGameplayAbilitySpecHandle Handle,
                                             const FGameplayAbilityActorInfo* ActorInfo, FGameplayAbilityActivationInfo ActivationInfo,
                                             const FGameplayEventData* TriggerEventData)
{
    auto OneSecond = Latent::Seconds(2.);
    auto Tag = FMyGameplayTags::Get();
    //auto* GEAccelerate =
    //ApplyGameplayEffectToOwner(Handle, ActorInfo, ActivationInfo, )
    //Spec->SetDuration(1., true);
    // TODO: which one?
    //Spec->SetByCallerTagMagnitudes[Tag.HasTorque] = 1.0;
    //Spec->SetByCallerTagMagnitudes[Tag.HasTorqueCCW] = 1.0;
    //ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, EffectSpecHandle);
    co_await OneSecond;
}

void UGA_LookAt::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
    Super::OnAvatarSet(ActorInfo, Spec);
    check(IsValid(GE_TorqueCCW))
    check(IsValid(GE_TorqueCW))
}
