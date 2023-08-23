#include "GameplayAbilitySystem/GA_Embrace.h"

#include "MyGameplayTags.h"
#include "GameplayAbilitySystem/MyAbilitySystemComponent.h"
#include "UE5Coro/LatentAwaiters.h"

UGA_Embrace::UGA_Embrace()
{
    const auto& Tag = FMyGameplayTags::Get();
    AbilityTags.AddTag(Tag.AbilityEmbrace);
}

FAbilityCoroutine UGA_Embrace::ExecuteAbility(FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if(!CommitAbility(Handle, ActorInfo, ActivationInfo))
        co_await Latent::Cancel();

    auto* MyPawn = Cast<ICanEmbrace>(ActorInfo->AvatarActor);
    check(MyPawn)

    MyPawn->GetOnOverlapEmbraceSphere().AddDynamic(this, &UGA_Embrace::MaybeStartEmbracing);

    // TODO: show awaiting embrace in UI
    
    co_await Latent::Until([this] { return bStartEmbracing; });

    auto* ASC = UMyAbilitySystemComponent::Get(ActorInfo);
    const auto& Tag = FMyGameplayTags::Get();

    // TODO: blend pose depending on `Span`
    ASC->AddPoseCue(Tag.CuePoseEmbrace);
    // TODO: fully inelastic collision
    //  ... including change of rotational speed

    co_await UntilReleased();
    ASC->RemovePoseCue(Tag.CuePoseEmbrace);

    // TODO: fly off depending on rotation
}

void UGA_Embrace::MaybeStartEmbracing(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if(OtherActor->Implements<UCanBeEmbraced>())
    {
        auto* Object = Cast<ICanBeEmbraced>(OtherActor);
        
        // TODO: check if the pawn has attributes/equipment that allow to embrace the other actor, given it's
        // * size
        // * relative surface velocity (rotational velocity + body velocity relative to pawn)
        Span = FMath::Min(1., Object->GetRadius() / 150.);
        auto* MyPawn = Cast<AMyPawn_Humanoid>(CurrentActorInfo->AvatarActor);
        bStartEmbracing = true;
        MyPawn->GetOnOverlapEmbraceSphere().RemoveDynamic(this, &UGA_Embrace::MaybeStartEmbracing);
    }
}
