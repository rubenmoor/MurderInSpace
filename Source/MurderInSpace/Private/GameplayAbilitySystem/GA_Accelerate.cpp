#include "GameplayAbilitySystem/GA_Accelerate.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_NetworkSyncPoint.h"
#include "MyGameplayTags.h"
#include "Actors/MyCharacter.h"
#include "Actors/MyPawn.h"
#include "Modes/MyPlayerController.h"
#include "UE5Coro/LatentAwaiters.h"

UGA_Accelerate::UGA_Accelerate()
{
    const auto& Tag = FMyGameplayTags::Get();
    
    AbilityTags.AddTag(Tag.InputBindingAbilityAccelerate);
    
    ActivationOwnedTags.AddTag(Tag.AbilityAccelerate);
}

FAbilityCoroutine UGA_Accelerate::ExecuteAbility(FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if(!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        co_await Latent::Cancel();
    }
    co_await Latent::Seconds(0.2);
    const auto GE_Handle =
        ApplyGameplayEffectToOwner
            ( Handle
            , ActorInfo
            , ActivationInfo
            , GE_Accelerate->GetDefaultObject<UGameplayEffect>()
            , 1.
            );
    LocallyDo(ActorInfo, [] (AMyCharacter* MyCharacter)
    {
        auto* Orbit = MyCharacter->GetOrbit();
        Orbit->UpdateVisibility(true);
        Orbit->SpawnSplineMesh
            ( MyCharacter->GetTempSplineMeshColor()
            , ESplineMeshParentSelector::Temporary
            );
    });
    co_await UntilReleased();
    BP_RemoveGameplayEffectFromOwnerWithHandle(GE_Handle);
    LocallyDo(ActorInfo, [] (AMyCharacter* MyCharacter)
    {
        auto* Orbit = MyCharacter->GetOrbit();
        Orbit->UpdateVisibility(false);
        Orbit->DestroyTempSplineMeshes();
    });
}

void UGA_Accelerate::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
    Super::OnAvatarSet(ActorInfo, Spec);
    check(IsValid(GE_Accelerate))
}
