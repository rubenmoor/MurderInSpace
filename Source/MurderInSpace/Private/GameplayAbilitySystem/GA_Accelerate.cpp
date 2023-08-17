#include "GameplayAbilitySystem/GA_Accelerate.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_NetworkSyncPoint.h"
#include "MyGameplayTags.h"
#include "Spacebodies/MyPawn.h"
#include "GameplayAbilitySystem/GE_AccelerateFire.h"
#include "GameplayAbilitySystem/MyAbilitySystemComponent.h"
#include "UE5Coro/LatentAwaiters.h"

using namespace UE5Coro;

UGA_Accelerate::UGA_Accelerate()
{
    const auto& Tag = FMyGameplayTags::Get();
    
    AbilityTags.AddTag(Tag.AbilityAccelerate);
    AbilityTags.AddTag(Tag.BlockingTurn);
    
    GE_AccelerateFire = UGE_AccelerateFire::StaticClass();
}

FAbilityCoroutine UGA_Accelerate::ExecuteAbility(FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    UE_LOGFMT(LogMyGame, Display, "ExecuteAbility Accelerate");
    
    if(!CommitAbility(Handle, ActorInfo, ActivationInfo))
        co_await Latent::Cancel();

    if(auto* OnBlockingAbilityEnded = TurnBlocked(Handle, ActorInfo))
        co_await Latent::UntilDelegate(*OnBlockingAbilityEnded);
    
    auto* ASC = UMyAbilitySystemComponent::Get(ActorInfo);
    const auto& Tag = FMyGameplayTags::Get();

    ASC->AddGameplayCueUnlessExists(Tag.CueShowThrusters);

    if(ASC->AddPoseCue(Tag.CuePoseAccelerate))
        co_await Latent::UntilDelegate(ASC->OnAnimStateFullyBlended);

    LocallyControlledDo(ActorInfo, [] (AMyCharacter* MyCharacter)
    {
        auto* Orbit = MyCharacter->GetOrbit();
        Orbit->UpdateVisibility(true);
        Orbit->SpawnSplineMesh
            ( MyCharacter->GetTempSplineMeshColor()
            , ESplineMeshParentSelector::Temporary
            );
    });

    const auto GE_FireSpec   = MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, GE_AccelerateFire);
    const auto GE_FireHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, GE_FireSpec);
            
    co_await UntilReleased();

    RemoveActiveGameplayEffect(GE_FireHandle, *ActorInfo, ActivationInfo);
    
    LocallyControlledDo(ActorInfo, [] (AMyCharacter* MyCharacter)
    {
        auto* Orbit = MyCharacter->GetOrbit();
        Orbit->UpdateVisibility(false);
        Orbit->DestroyTempSplineMeshes();
    });

    if(ASC->RemovePoseCue(Tag.CuePoseAccelerate))
        co_await Latent::UntilDelegate(ASC->OnAnimStateFullyBlended);
    
    ASC->RemoveGameplayCue(Tag.CueShowThrusters);
}
