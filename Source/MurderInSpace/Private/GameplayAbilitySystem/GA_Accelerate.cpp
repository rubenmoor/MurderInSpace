#include "GameplayAbilitySystem/GA_Accelerate.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_NetworkSyncPoint.h"
#include "MyGameplayTags.h"
#include "Spacebodies/MyCharacter.h"
#include "Spacebodies/MyPawn.h"
#include "..\..\Public\GameplayAbilitySystem\GE_AcceleratePosition.h"
#include "GameplayAbilitySystem/GE_AccelerateFire.h"
#include "GameplayAbilitySystem/MyAbilitySystemComponent.h"
#include "Modes/MyPlayerController.h"
#include "UE5Coro/LatentAwaiters.h"

using namespace UE5Coro;

UGA_Accelerate::UGA_Accelerate()
{
    const auto& Tag = FMyGameplayTags::Get();
    
    AbilityTags.AddTag(Tag.InputBindingAbilityAccelerate);
    
    //ActivationOwnedTags.AddTag(Tag.AbilityAccelerate);
    
    GE_AcceleratePosition = UGE_AcceleratePosition::StaticClass();
    GE_AccelerateFire = UGE_AccelerateFire::StaticClass();
}

FAbilityCoroutine UGA_Accelerate::ExecuteAbility(FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if(!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        co_await Latent::Cancel();
    }
    const auto GE_Position =
        ApplyGameplayEffectToOwner
            ( Handle
            , ActorInfo
            , ActivationInfo
            , GE_AcceleratePosition->GetDefaultObject<UGameplayEffect>()
            , 1.
            );
    
    // TODO: wait for transition event
    co_await Latent::Seconds(0.2);
    
    auto* ASC = UMyAbilitySystemComponent::Get(ActorInfo);
    const auto& Tag = FMyGameplayTags::Get();
    ASC->AddGameplayCueLocal(Tag.LocalCueAccelerateFire, FGameplayCueParameters());
    
    const auto GE_Fire =
        ApplyGameplayEffectToOwner
            ( Handle
            , ActorInfo
            , ActivationInfo
            , GE_AccelerateFire->GetDefaultObject<UGameplayEffect>()
            , 1.
            );
            
    co_await UntilReleased();
    
    ASC->RemoveActiveGameplayEffect(GE_Fire);
    ASC->RemoveActiveGameplayEffect(GE_Position);
    
    co_await Latent::Seconds(0.2);
    ASC->RemoveGameplayCueLocal(Tag.LocalCueAccelerateFire, FGameplayCueParameters());
}
