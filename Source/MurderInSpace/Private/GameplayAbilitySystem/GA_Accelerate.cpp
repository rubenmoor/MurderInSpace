#include "GameplayAbilitySystem/GA_Accelerate.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_NetworkSyncPoint.h"
#include "MyGameplayTags.h"
#include "Engine/LocalPlayer.h"
#include "Spacebodies/MyPawn.h"
#include "GameplayAbilitySystem/GE_AccelerateFire.h"
#include "GameplayAbilitySystem/MyAbilitySystemComponent.h"
#include "HUD/MyHUD.h"
#include "HUD/UW_MyAbilities.h"
#include "Spacebodies/MyCharacter.h"
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
    if(!CommitAbility(Handle, ActorInfo, ActivationInfo))
        co_await Latent::Cancel();

    if(auto* OnBlockingAbilityEnded = TurnBlocked(Handle, ActorInfo))
        co_await Latent::UntilDelegate(*OnBlockingAbilityEnded);
    
    auto* ASC = UMyAbilitySystemComponent::Get(ActorInfo);
    const auto& Tag = FMyGameplayTags::Get();

    LocallyControlledDo(ActorInfo, [] (const FLocalPlayerContext& LPC)
    {
        const auto& Tag = FMyGameplayTags::Get();
        LPC.GetHUD<AMyHUD>()->WidgetHUD->WidgetAbilities->SetVisibilityArrow(Tag.AbilityAccelerate, true);
    });

    ASC->AddGameplayCueUnlessExists(Tag.CueShowThrusters);

    if(ASC->AddPoseCue(Tag.CuePoseAccelerate))
        co_await Latent::UntilDelegate(ASC->OnAnimStateFullyBlended);

    LocallyControlledDo(ActorInfo, [] (const FLocalPlayerContext& LPC)
    {
        
        auto* Orbit = LPC.GetPawn<AMyCharacter>()->GetOrbit();
        Orbit->UpdateVisibility(true);
        Orbit->SpawnSplineMesh
            ( LPC.GetPawn<AMyCharacter>()->GetTempSplineMeshColor()
            , ESplineMeshParentSelector::Temporary
            );
    });

    const auto GE_FireSpec   = MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, GE_AccelerateFire);
    const auto GE_FireHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, GE_FireSpec);
            
    co_await UntilReleased();

    RemoveActiveGameplayEffect(GE_FireHandle, *ActorInfo, ActivationInfo);
    
    LocallyControlledDo(ActorInfo, [] (const FLocalPlayerContext& LPC)
    {
        auto* Orbit = LPC.GetPawn<AMyCharacter>()->GetOrbit();
        Orbit->UpdateVisibility(false);
        Orbit->DestroyTempSplineMeshes();
    });

    if(ASC->RemovePoseCue(Tag.CuePoseAccelerate))
        co_await Latent::UntilDelegate(ASC->OnAnimStateFullyBlended);
    
    ASC->RemoveGameplayCue(Tag.CueShowThrusters);
    
    LocallyControlledDo(ActorInfo, [] (const FLocalPlayerContext& LPC)
    {
        const auto& Tag = FMyGameplayTags::Get();
        LPC.GetHUD<AMyHUD>()->WidgetHUD->WidgetAbilities->SetVisibilityArrow(Tag.AbilityAccelerate, false);
    });

}
