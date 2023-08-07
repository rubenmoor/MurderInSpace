#include "GameplayAbilitySystem/GA_Accelerate.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_NetworkSyncPoint.h"
#include "MyGameplayTags.h"
#include "Spacebodies/MyPawn.h"
#include "GameplayAbilitySystem/GE_AcceleratePosition.h"
#include "GameplayAbilitySystem/GE_AccelerateFire.h"
#include "GameplayAbilitySystem/MyAbilitySystemComponent.h"
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
    auto* ASC = UMyAbilitySystemComponent::Get(ActorInfo);
    const auto& Tag = FMyGameplayTags::Get();

    ASC->AddGameplayCue(Tag.CueAccelerateShowThrusters);
    ASC->AddGameplayCue(Tag.CuePoseAccelerate);

    co_await Latent::UntilDelegate(ASC->OnStateFullyBlended);
    //co_await Latent::Seconds(0.2);

    LocallyControlledDo(ActorInfo, [] (AMyCharacter* MyCharacter)
    {
        auto* Orbit = MyCharacter->GetOrbit();
		Orbit->UpdateVisibility(true);
		Orbit->SpawnSplineMesh
			( MyCharacter->GetTempSplineMeshColor()
			, ESplineMeshParentSelector::Temporary
			);
    });
    //ASC->AddGameplayCueLocal(Tag.LocalCueAccelerateFire, FGameplayCueParameters());
    
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
    ASC->RemoveGameplayCue(Tag.CuePoseAccelerate);
    
    LocallyControlledDo(ActorInfo, [] (AMyCharacter* MyCharacter)
    {
        auto* Orbit = MyCharacter->GetOrbit();
        Orbit->UpdateVisibility(false);
        Orbit->DestroyTempSplineMeshes();
    });
    
    co_await Latent::UntilDelegate(ASC->OnStateFullyBlended);
    ASC->RemoveGameplayCue(Tag.CueAccelerateShowThrusters);
}
