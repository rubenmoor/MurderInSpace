#include "GameplayAbilitySystem/GA_LookAt.h"

#include "GameplayAbilitySystem/MyAttributes.h"
#include "MyGameplayTags.h"
#include "UE5Coro/LatentAwaiters.h"

using namespace UE5Coro;
using namespace UE5Coro::GAS;

UGA_LookAt::UGA_LookAt()
{
    const auto Tag = FMyGameplayTags::Get();
    InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
    
    AbilityTags.AddTag(Tag.AbilityLookAt);
    AbilityTags.AddTag(Tag.InputBindingAbilityLookAt);
    
    // cancel any active LookAt ability
    CancelAbilitiesWithTag.AddTag(Tag.AbilityLookAt);
}

FAbilityCoroutine UGA_LookAt::ExecuteAbility(FGameplayAbilitySpecHandle Handle,
                                             const FGameplayAbilityActorInfo* ActorInfo, FGameplayAbilityActivationInfo ActivationInfo,
                                             const FGameplayEventData* TriggerEventData)
{
    if(!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        co_await Latent::Cancel();
    }
    // TODO: calculate timing depending on current Theta, Omega, and Alpha
    auto T2 = Latent::Seconds(2.);
    auto Tag = FMyGameplayTags::Get();
    auto Torque1 = MakeOutgoingGameplayEffectSpec(GE_TorqueCCW);
    //ApplyGameplayEffectToOwner(Handle, ActorInfo, ActivationInfo, )
    Torque1.Data.Get()->SetDuration(1., true);
    auto Torque2 = MakeOutgoingGameplayEffectSpec(GE_TorqueCW);
    Torque2.Data.Get()->SetDuration(1., true);
    
    ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, Torque1);
    co_await T2;
    ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, Torque2);
}

void UGA_LookAt::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
    Super::OnAvatarSet(ActorInfo, Spec);
    check(GE_TorqueCW)
    check(GE_TorqueCCW)
}
