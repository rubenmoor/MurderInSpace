﻿#include "GameplayAbilitySystem/GA_Embrace.h"

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
    AbilityTags.AddTag(Tag.BlockingTurn);
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
    if(!ActorInfo->AvatarActor->Implements<UHasCollision>())
    {
        UE_LOGFMT(LogMyGame, Error, "AvatarActor does not implement IHasCollision");
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
    
    //  TODO: include change of rotational speed

    // TODO: check if the pawn has attributes/equipment that allow to embrace the other actor, given it's
    // * size
    // * relative surface velocity (rotational velocity + body velocity relative to pawn)
    Span = FMath::Min(1., Cast<ICanBeEmbraced>(OtherActor)->GetRadius() / 150.);
        
    // fully inelastic collision (plastic collision, K=0)
	// calculation for 3 dimensions
	// https://en.wikipedia.org/wiki/Inelastic_collision
    
	auto* Orbit = Cast<IHasOrbit>(ActorInfo->AvatarActor)->GetOrbit();
	auto* OtherOrbit = Cast<IHasOrbit>(OtherActor)->GetOrbit();
	
    const double MyMass = Cast<IHasCollision>(ActorInfo->AvatarActor)->GetCollisionComponent()->GetMyMass();
    const double OtherMass = Cast<IHasCollision>(OtherActor)->GetCollisionComponent()->GetMyMass();
    const FVector VecV1 = Orbit->GetVecVelocity();
    const FVector VecV2 = OtherOrbit->GetVecVelocity();
 	const double J = MyMass * OtherMass / (MyMass + OtherMass) * (VecV2 - VecV1).Dot(VecN);
	const FVector VecDeltaV1 = J / MyMass * VecN;
	const FVector VecDeltaV2 = -J / OtherMass * VecN;

    const auto* GS = GetWorld()->GetGameState<AMyGameState>();
    Orbit->Update(VecDeltaV1, GS->RP_Physics);
    OtherOrbit->Update(VecDeltaV2, GS->RP_Physics);

    co_await UntilReleased();

    // TODO: fly off depending on rotation
    OtherActor = nullptr;
}

void UGA_Embrace::MaybeStartEmbracing(UPrimitiveComponent* OverlappedComponent, AActor* InOtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if(InOtherActor->Implements<UCanBeEmbraced>())
    {
        check(InOtherActor->Implements<UHasCollision>())
        
        OtherActor = InOtherActor;
        VecN = SweepResult.ImpactNormal;
        bStartEmbracing = true;
        
        auto* MyPawn = Cast<AMyPawn_Humanoid>(CurrentActorInfo->AvatarActor);
        MyPawn->GetOnOverlapEmbraceSphere().RemoveDynamic(this, &UGA_Embrace::MaybeStartEmbracing);
    }
}
