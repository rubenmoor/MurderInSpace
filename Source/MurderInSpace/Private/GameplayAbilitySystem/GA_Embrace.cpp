#include "GameplayAbilitySystem/GA_Embrace.h"

#include "MyGameplayTags.h"
#include "GameplayAbilitySystem/MyAbilitySystemComponent.h"
#include "HUD/MyHUD.h"
#include "HUD/UW_MyAbilities.h"
#include "Spacebodies/MyPawn_Humanoid.h"
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

    const auto& Tag = FMyGameplayTags::Get();
    
    if(auto* OnBlockingAbilityEnded = TurnBlocked(Handle, ActorInfo))
    {
        LocallyControlledDo(ActorInfo, [&Tag] (const FLocalPlayerContext& LPC)
        {
            LPC.GetHUD<AMyHUD>()->WidgetHUD->WidgetAbilities->SetBordered(Tag.AbilityEmbrace, true);
        });
        co_await Latent::UntilDelegate(*OnBlockingAbilityEnded);
        LocallyControlledDo(ActorInfo, [&Tag] (const FLocalPlayerContext& LPC)
        {
            LPC.GetHUD<AMyHUD>()->WidgetHUD->WidgetAbilities->SetBordered(Tag.AbilityEmbrace, false);
        });
    }

    auto* ASC = UMyAbilitySystemComponent::Get(ActorInfo);

    if(!ActorInfo->AvatarActor->Implements<UCanEmbrace>())
    {
        UE_LOGFMT(LogMyGame, Error, "AvatarActor does not implement ICanEmbrace");
        co_await Latent::Cancel();
    }
    
    auto* MyPawn = Cast<ICanEmbrace>(ActorInfo->AvatarActor);

    MyPawn->GetOnOverlapEmbraceSphere().AddDynamic(this, &UGA_Embrace::MaybeStartEmbracing);

    LocallyControlledDo(ActorInfo, [&Tag] (const FLocalPlayerContext& LPC)
    {
       LPC.GetHUD<AMyHUD>()->WidgetHUD->WidgetAbilities->SetVisibilityArrow(Tag.AbilityEmbrace, true);
    });
    
    OnGameplayAbilityEnded.AddLambda([this, &ActorInfo, &Tag, ASC] (UGameplayAbility*)
    {
        LocallyControlledDo(ActorInfo, [&Tag] (const FLocalPlayerContext& LPC)
        {
           LPC.GetHUD<AMyHUD>()->WidgetHUD->WidgetAbilities->SetVisibilityArrow(Tag.AbilityEmbrace, false);
        });
        ASC->RemovePoseCue(Tag.CuePoseEmbracePrepare);
        ASC->RemovePoseCue(Tag.CuePoseEmbraceExecute);
        OnGameplayAbilityEnded.Clear();
    });

    ASC->AddPoseCue(Tag.CuePoseEmbracePrepare);
    
    co_await Latent::Until([this] { return bStartEmbracing || bReleased; });
    if(bReleased)
    {
        bReleased = false;
        MyPawn->GetOnOverlapEmbraceSphere().RemoveDynamic(this, &UGA_Embrace::MaybeStartEmbracing);
        co_await Latent::Cancel();
    }

    // TODO: blend pose depending on `Span`
    ASC->AddPoseCue(Tag.CuePoseEmbraceExecute);
    // TODO: fully inelastic collision
    //  ... including change of rotational speed

    co_await UntilReleased();

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
        MyPawn->GetOnOverlapEmbraceSphere().RemoveDynamic(this, &UGA_Embrace::MaybeStartEmbracing);
        bStartEmbracing = true;
    }
}
