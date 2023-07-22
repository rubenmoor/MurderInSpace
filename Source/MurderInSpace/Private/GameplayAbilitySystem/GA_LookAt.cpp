#include "GameplayAbilitySystem/GA_LookAt.h"

#include "GameplayAbilitySystem/MyAttributes.h"
#include "MyGameplayTags.h"
#include "UE5Coro/LatentAwaiters.h"

using namespace UE5Coro;
using namespace UE5Coro::GAS;

UGA_LookAt::UGA_LookAt()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
    // TODO: set policy to cancel other active abilities of same type
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
    auto Accelerate = MakeOutgoingGameplayEffectSpec(GE_TorqueCCW);
    //ApplyGameplayEffectToOwner(Handle, ActorInfo, ActivationInfo, )
    Accelerate.Data.Get()->SetDuration(1., true);
    auto Decelerate = MakeOutgoingGameplayEffectSpec(GE_TorqueCW);
    Decelerate.Data.Get()->SetDuration(1., true);
    
    ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, Accelerate);
    co_await T2;
    ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, Decelerate);
}

void UGA_LookAt::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
    Super::OnAvatarSet(ActorInfo, Spec);
    check(GE_TorqueCW)
    check(GE_TorqueCCW)
}
