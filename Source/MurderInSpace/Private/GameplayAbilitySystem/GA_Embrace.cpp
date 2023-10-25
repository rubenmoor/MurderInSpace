#include "GameplayAbilitySystem/GA_Embrace.h"

#include "MyGameplayTags.h"
#include "Components/SphereComponent.h"
#include "GameplayAbilitySystem/MyAbilitySystemComponent.h"
#include "GameplayAbilitySystem/GE_MoveForward.h"
#include "GameplayAbilitySystem/GE_MoveBackward.h"
#include "GameplayAbilitySystem/GE_RotateCCW.h"
#include "GameplayAbilitySystem/GE_RotateCW.h"
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

    MyPawn->GetEmbraceSphere()->OnComponentBeginOverlap.AddDynamic(this, &UGA_Embrace::MaybeStartEmbracing);

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

    // check if already overlapping an actor that qualifies ...
    
    TArray<AActor*> OverlappingActors;
    MyPawn->GetEmbraceSphere()->GetOverlappingActors(OverlappingActors);
    AActor* ClosestActor = nullptr;
    double Distance = MyPawn->GetEmbraceSphere()->GetScaledSphereRadius() + 1.;
    for(auto* OverlappingActor : OverlappingActors)
    {
        const double NewDistance =
            (OverlappingActor->GetActorLocation() - MyPawn->GetEmbraceSphere()->GetComponentLocation()).Length();
        if(NewDistance < Distance)
        {
            ClosestActor = OverlappingActor;
            Distance = NewDistance;
        }
    }
    if(ClosestActor)
        MaybeStartEmbracing(nullptr, ClosestActor, nullptr, 0, false, FHitResult());

    // ... or wait until an actor overlaps (or the input is released)
    
    co_await Latent::Until([this] { return bStartEmbracing || bReleased; });
    if(bReleased)
    {
        bReleased = false;
        MyPawn->GetEmbraceSphere()->OnComponentBeginOverlap.RemoveDynamic(this, &UGA_Embrace::MaybeStartEmbracing);
        co_await Latent::Cancel();
    }

    // reset
    bStartEmbracing = false;

    // rotate and approach the other actor

    const double AngleStart = CalcAngle(ActorInfo->AvatarActor.Get(), OtherActor);
    const auto SpecTorque =
        MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, AngleStart > 0 ? GE_RotateCW : GE_RotateCCW);
    auto HandleTorque = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecTorque);
    bRotating = true;

    const double EmbraceDistanceStart = CalcEmbraceDistance(ActorInfo->AvatarActor.Get(), OtherActor);
    const auto SpecForward =
        MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, EmbraceDistanceStart > 0. ? GE_MoveForward : GE_MoveBackward);
    // auto HandleForwardMovement =
    //     ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecForward);
    bMovingForward = true;

    // TODO: blend pose depending on `Span`
    ASC->AddPoseCue(Tag.CuePoseEmbraceExecute);
    
    // co_await Latent::Until([this, ActorInfo, ActivationInfo, AngleStart, EmbraceDistanceStart, HandleTorque, HandleForwardMovement]
    co_await Latent::Until([this, ActorInfo, ActivationInfo, AngleStart, EmbraceDistanceStart, HandleTorque]
    {
        const bool bRotationFinished = AngleStart * CalcAngle(ActorInfo->AvatarActor.Get(), OtherActor) < 0;
        const bool bForwardMovementFinished = EmbraceDistanceStart * CalcEmbraceDistance(ActorInfo->AvatarActor.Get(), OtherActor) < 0;
        if(bRotating && bRotationFinished)
        {
            RemoveActiveGameplayEffect(HandleTorque, *ActorInfo, ActivationInfo);
            bRotating = false;
        }
        // if(bMovingForward && bForwardMovementFinished)
        // {
        //     RemoveActiveGameplayEffect(HandleForwardMovement, *ActorInfo, ActivationInfo);
        //     bMovingForward = false;
        // }
        bMovingForward = false;
        return !(bRotating || bMovingForward);
    });

    UE_LOGFMT(LogMyGame, Warning, "Embrace: finished positioning");
    
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

 	const FVector VecJ = (VecV2 - VecV1) / (MyMass + OtherMass) ;
	const FVector VecDeltaV1 = VecJ * OtherMass;
	const FVector VecDeltaV2 = -VecJ * MyMass;

    const auto* GS = GetWorld()->GetGameState<AMyGameState>();

    const FVector VecLoc1 = ActorInfo->AvatarActor->GetActorLocation();
    const FVector VecLoc2 = OtherActor->GetActorLocation();
    const FVector VecCoMLoc = (VecLoc1 * MyMass + VecLoc2 * OtherMass) / (MyMass + OtherMass);
    const FVector Offset1 = VecLoc1 - VecCoMLoc;
    const FVector Offset2 = VecLoc2 - VecCoMLoc;

    Orbit->Update(VecDeltaV1, Offset1, 0., GS->RP_Physics);
    OtherOrbit->Update(VecDeltaV2, Offset2, 0., GS->RP_Physics);
    UE_LOGFMT(LogMyGame, Warning, "Embrace begin: VecDeltaV1: {V1}, VecDeltaV2: {V2}", VecDeltaV1.ToString(), VecDeltaV2.ToString());
    UE_LOGFMT(LogMyGame, Warning, "VecVelocity1: {V1}, VecVelocity2: {V2}", Orbit->GetVecVelocity().ToString(), OtherOrbit->GetVecVelocity().ToString());

    co_await UntilReleased();

    // a small momentum to push the asteroid away when letting loose
    const FVector VecNPush = (OtherActor->GetActorLocation() - ActorInfo->AvatarActor->GetActorLocation()).GetSafeNormal();
    const FVector VecDeltaV1Push = SmallPushAmount / MyMass * VecNPush;
    const FVector VecDeltaV2Push = SmallPushAmount / OtherMass * -VecNPush;
    Orbit->Update(VecDeltaV1Push, GS->RP_Physics);
    OtherOrbit->Update(VecDeltaV2Push, GS->RP_Physics);
    UE_LOGFMT(LogMyGame, Warning, "Embrace end: Push: VecDeltaV1: {V1}, VecDeltaV2: {V2}", VecDeltaV1Push.ToString(), VecDeltaV2Push.ToString());
    
    OtherActor = nullptr;
}

void UGA_Embrace::MaybeStartEmbracing(UPrimitiveComponent* _OverlappedComponent, AActor* InOtherActor,
    UPrimitiveComponent* _OtherComp, int32 _OtherBodyIndex, bool _bFromSweep, const FHitResult& _SweepResult)
{
    /*
     * don't use any other parameter then `InOtherActor`
     */
    
    if(InOtherActor->Implements<UCanBeEmbraced>())
    {
        check(InOtherActor->Implements<UHasCollision>())
        check(InOtherActor->Implements<UHasMesh>())
        
        OtherActor = InOtherActor;
        bStartEmbracing = true;
        
        auto* MyPawn = Cast<AMyPawn_Humanoid>(CurrentActorInfo->AvatarActor);
        MyPawn->GetEmbraceSphere()->OnComponentBeginOverlap.RemoveDynamic(this, &UGA_Embrace::MaybeStartEmbracing);
    }
}

double UGA_Embrace::CalcAngle(const AActor* Actor1, const AActor* Actor2)
{
    const FVector VecForward = Actor1->GetActorForwardVector();
    const FVector VecAim = Actor2->GetActorLocation() - Actor1->GetActorLocation();
    return FMath::Atan2(VecForward.Y, VecForward.X) - FMath::Atan2(VecAim.Y, VecAim.X);
}

double UGA_Embrace::CalcEmbraceDistance(const AActor* Actor1, const AActor* Actor2)
{
    return
          (Actor2->GetActorLocation() - Actor1->GetActorLocation()).Length()
        - Cast<IHasMesh>(Actor2)->GetBounds().SphereRadius
        + EmbraceDistanceModifier;
}
