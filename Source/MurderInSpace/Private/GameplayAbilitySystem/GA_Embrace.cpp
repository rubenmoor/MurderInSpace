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

    bStartEmbracing = false;
    
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
    VecN = (VecV2 - VecV1).GetSafeNormal();
 	const double J = MyMass * OtherMass / (MyMass + OtherMass) * (VecV2 - VecV1).Dot(VecN);
	const FVector VecDeltaV1 = J / MyMass * VecN;
	const FVector VecDeltaV2 = -J / OtherMass * VecN;

    const auto* GS = GetWorld()->GetGameState<AMyGameState>();

    const FVector VecLoc1 = ActorInfo->AvatarActor->GetActorLocation();
    const FVector VecLoc2 = OtherActor->GetActorLocation();
    const FVector VecCoM = (VecLoc1 * MyMass + VecLoc2 * OtherMass) / (MyMass + OtherMass);
    const FVector Offset1 = VecLoc1 - VecCoM;
    const FVector Offset2 = VecLoc2 - VecCoM;
    Orbit->Update(VecDeltaV1, Offset1, 0., GS->RP_Physics);
    OtherOrbit->Update(VecDeltaV2, Offset2, 0., GS->RP_Physics);
    UE_LOGFMT(LogMyGame, Warning, "VecDeltaV1: {V1}, VecDeltaV2: {V2}", VecDeltaV1.ToString(), VecDeltaV2.ToString());

    co_await UntilReleased();

    // a small momentum to push the asteroid away when letting loose
    const double SmallPush = 10.;
    const FVector VecNPush = (OtherActor->GetActorLocation() - ActorInfo->AvatarActor->GetActorLocation()).GetSafeNormal();
    const FVector VecDeltaV1Push = SmallPush / MyMass * VecNPush;
    const FVector VecDeltaV2Push = SmallPush / OtherMass * -VecNPush;
    Orbit->Update(VecDeltaV1Push, GS->RP_Physics);
    OtherOrbit->Update(VecDeltaV2Push, GS->RP_Physics);
    
    OtherActor = nullptr;
}

void UGA_Embrace::MaybeStartEmbracing(UPrimitiveComponent* OverlappedComponent, AActor* InOtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    UE_LOGFMT(LogMyGame, Warning, "MaybeStartEmbracing: {NAME}", InOtherActor->GetName());
    if(InOtherActor->Implements<UCanBeEmbraced>())
    {
        check(InOtherActor->Implements<UHasCollision>())
        
        OtherActor = InOtherActor;
        const FVector VecLoc1 = OverlappedComponent->GetOwner()->GetActorLocation();
        const FVector VecLoc2 = OtherActor->GetActorLocation();
        bStartEmbracing = true;
        
        auto* MyPawn = Cast<AMyPawn_Humanoid>(CurrentActorInfo->AvatarActor);
        MyPawn->GetOnOverlapEmbraceSphere().RemoveDynamic(this, &UGA_Embrace::MaybeStartEmbracing);
    }
}
