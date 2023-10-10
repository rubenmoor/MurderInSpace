#include "GameplayAbilitySystem/GA_LookAt.h"

#include "GameplayAbilitySystem/MyAttributes.h"
#include "MyGameplayTags.h"
#include "Engine/LocalPlayer.h"
#include "GameplayAbilitySystem/GE_TorqueCCW.h"
#include "GameplayAbilitySystem/GE_TorqueCW.h"
#include "GameplayAbilitySystem/MyAbilitySystemComponent.h"
#include "HUD/MyHUD.h"
#include "HUD/UW_MyAbilities.h"
#include "Lib/FunctionLib.h"
#include "Logging/StructuredLog.h"
#include "Modes/MyGameInstance.h"
#include "Spacebodies/MyCharacter.h"
#include "UE5Coro/LatentAwaiters.h"

using namespace UE5Coro;
using namespace UE5Coro::GAS;

UGA_LookAt::UGA_LookAt()
{
    const auto& Tag = FMyGameplayTags::Get();

    AbilityTags.AddTag(Tag.AbilityLookAt);
    AbilityTags.AddTag(Tag.BlockingTurn);
    
    FAbilityTriggerData TriggerData;
    TriggerData.TriggerTag = Tag.AbilityLookAt;
    AbilityTriggers.Add(TriggerData);

    GE_TorqueCCW = UGE_TorqueCCW::StaticClass();
    GE_TorqueCW = UGE_TorqueCW::StaticClass();
}

FAbilityCoroutine UGA_LookAt::ExecuteAbility(FGameplayAbilitySpecHandle Handle,
                                             const FGameplayAbilityActorInfo* ActorInfo, FGameplayAbilityActivationInfo ActivationInfo,
                                             const FGameplayEventData* TriggerEventData)
{
    checkf(TriggerEventData, TEXT("Use UAbilitySystemComponent::SendGameplayEvent to activate this ability"))
    
    if(!CommitAbility(Handle, ActorInfo, ActivationInfo))
        co_await Latent::Cancel();

    // get the payload before any latent action, `*TriggerEventData` gets garbage collected/removed from stack
    const float Payload = TriggerEventData->EventMagnitude;
    check(FMath::Abs(Payload) <= TWO_PI)
    
    const auto& Tag = FMyGameplayTags::Get();

    UE_LOGFMT(LogMyGame, Warning, "UGA_LookAt: Begin");
    OnGameplayAbilityEnded.AddLambda([this] (UGameplayAbility*)
    {
        UE_LOGFMT(LogMyGame, Warning, "UGA_LookAt: Ended");
        OnGameplayAbilityEnded.Clear();
    });
    
    if(auto* OnBlockingAbilityEnded = TurnBlocked(Handle, ActorInfo))
    {
        LocallyControlledDo(ActorInfo, [&Tag] (const FLocalPlayerContext& LPC)
        {
            LPC.GetHUD<AMyHUD>()->WidgetHUD->WidgetAbilities->SetBordered(Tag.AbilityLookAt, true);
        });
        co_await Latent::UntilDelegate(*OnBlockingAbilityEnded);
        LocallyControlledDo(ActorInfo, [&Tag] (const FLocalPlayerContext& LPC)
        {
            LPC.GetHUD<AMyHUD>()->WidgetHUD->WidgetAbilities->SetBordered(Tag.AbilityLookAt, false);
        });
    }
    
    auto* ASC = UMyAbilitySystemComponent::Get(ActorInfo);

    const float OmegaMaxByAttribute = ASC->GetNumericAttribute(UAttrSetAcceleration::GetOmegaMaxAttribute());
    const float Alpha = ASC->GetNumericAttribute(UAttrSetAcceleration::GetTorqueMaxAttribute());
    auto* MyPawn = Cast<AMyPawn>(ActorInfo->OwnerActor);
    
    const auto LookAtAngle =
        UFunctionLib::WrapRadians
            (static_cast<float>(MyPawn->GetActorQuat().GetTwistAngle(FVector::UnitZ())) + Payload);

    float DeltaTheta = NewDeltaTheta(MyPawn, LookAtAngle, Alpha, MyPawn->GetOmega(), Payload);
    
    while(true)
    {
        const auto Omega = static_cast<float>(MyPawn->GetOmega());
        const float OmegaAbs = FMath::Abs(Omega);
        // make sure that a real |Omega| > OmegaMax (by attribute) doesn't cause a problem
        const float OmegaMax = FMath::Max(OmegaMaxByAttribute, OmegaAbs);
        const float DeltaThetaAbs = FMath::Abs(DeltaTheta);
        const float SignDeltaTheta = FMath::Sign(DeltaTheta);

        // the breaking distance assuming negative pose
        const float BreakingDistanceShort =
            FMath::Pow(OmegaAbs, 2) / 2. / Alpha;

        // the breaking distance assuming change to negative pose necessary
        const float BreakingDistanceMedium =
              TransitionTime * OmegaAbs
            + BreakingDistanceShort;
        
        // the long breaking distance: enough time to switch to idle pose and then to negative pose
        const float BreakingDistanceLong =
              TransitionTime * OmegaAbs // time to transition to idle pose
            + BreakingDistanceMedium;

        // acc = acceleration, dec = decaleration
        const FGameplayTag CuePoseAcc = DeltaTheta > 0 ? Tag.CuePoseTorqueCCW : Tag.CuePoseTorqueCW;
        const FGameplayTag CuePoseDec = DeltaTheta > 0 ? Tag.CuePoseTorqueCW  : Tag.CuePoseTorqueCCW;
        const auto SpecTorqueAcc =
            MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, DeltaTheta > 0 ? GE_TorqueCCW : GE_TorqueCW);
        const auto SpecTorqueDec =
            MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, DeltaTheta > 0 ? GE_TorqueCW : GE_TorqueCCW);

        if(Omega == 0.)
        {
            if(DeltaTheta == 0.)
            {
                // ability ends
                ASC->RemoveGameplayCueIfExists(Tag.CueShowThrusters);
                LocallyControlledDo(ActorInfo, [&Tag] (const FLocalPlayerContext& LPC)
                {
                    LPC.GetHUD<AMyHUD>()->WidgetHUD->WidgetAbilities->SetVisibilityArrow(Tag.AbilityLookAt, false);
                });
                OnLookAtEnded.Broadcast(this);
                break;
            }
            else
            {
                // rotational acceleration about to start (or there was a Case 5)
                ASC->AddGameplayCueUnlessExists(Tag.CueShowThrusters);
                LocallyControlledDo(ActorInfo, [&Tag] (const FLocalPlayerContext& LPC)
                {
                    LPC.GetHUD<AMyHUD>()->WidgetHUD->WidgetAbilities->SetVisibilityArrow(Tag.AbilityLookAt, true);
                });
            }
        }
        
        if(Omega * DeltaTheta < 0. || DeltaThetaAbs < BreakingDistanceShort || (DeltaThetaAbs < BreakingDistanceMedium && !ASC->HasMatchingGameplayTag(CuePoseDec)))
        {
            // case 5: turn around 
            const bool b1 = ASC->RemovePoseCue(Omega > 0. ? Tag.CuePoseTorqueCCW : Tag.CuePoseTorqueCW );
            const bool b2 = ASC->AddPoseCue   (Omega > 0. ? Tag.CuePoseTorqueCW  : Tag.CuePoseTorqueCCW);
            if(b1 || b2)
            {
                RemoveActiveGameplayEffect(TorqueHandle, *ActorInfo, ActivationInfo);
                co_await Latent::Seconds(TransitionTime);
            }
            const auto SpecTorque =
                MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, Omega > 0 ? GE_TorqueCW : GE_TorqueCCW);
            SetGameplayEffectTorque(Handle, ActorInfo, ActivationInfo, SpecTorque);
            co_await Latent::Seconds(OmegaAbs / Alpha);
            RemoveActiveGameplayEffect(TorqueHandle, *ActorInfo, ActivationInfo);
            
            MyPawn->SetOmega(0.);
            // have DeltaTheta depend on actor position; fine because Omega == 0
            DeltaTheta = UFunctionLib::WrapRadians(LookAtAngle - MyPawn->GetActorQuat().GetTwistAngle(FVector::UnitZ()));
        }
        else if(DeltaThetaAbs == BreakingDistanceShort && ASC->HasMatchingGameplayTag(CuePoseDec))
        {
            // Case 1: at short breaking distance, already in negative pose: decelerate until stop
            
            SetGameplayEffectTorque(Handle, ActorInfo, ActivationInfo, SpecTorqueDec);
            co_await Latent::Seconds(OmegaAbs / Alpha);
            RemoveActiveGameplayEffect(TorqueHandle, *ActorInfo, ActivationInfo);

            MyPawn->SetOmega(0.);
            DeltaTheta = NewDeltaTheta(MyPawn, LookAtAngle, Alpha, 0., 0.);

            ASC->RemovePoseCue(CuePoseDec);
            co_await Latent::Seconds(TransitionTime);
            continue;
        }
        else if(DeltaThetaAbs < BreakingDistanceMedium && ASC->HasMatchingGameplayTag(CuePoseDec))
        {
            // Case 2a: uniform rotation in correct pose, then Case 1
            
            RemoveActiveGameplayEffect(TorqueHandle, *ActorInfo, ActivationInfo);
            const float DeltaTime = (DeltaThetaAbs - BreakingDistanceShort) / OmegaAbs;
            co_await Latent::Seconds(DeltaTime);
            DeltaTheta = NewDeltaTheta(MyPawn, LookAtAngle, Alpha, Omega, SignDeltaTheta * BreakingDistanceShort);
        }
        else if(DeltaThetaAbs == BreakingDistanceMedium)
        {
            // Case 2b: at medium breaking distance: uniform rotation during pose change and then Case 1
            
            RemoveActiveGameplayEffect(TorqueHandle, *ActorInfo, ActivationInfo);
            ASC->RemovePoseCue(CuePoseAcc);
            ASC->AddPoseCue(CuePoseDec);
            co_await Latent::Seconds(TransitionTime);
            DeltaTheta = NewDeltaTheta(MyPawn, LookAtAngle, Alpha, Omega, SignDeltaTheta * BreakingDistanceShort);
            continue;
        }
        else if(DeltaThetaAbs < BreakingDistanceLong && !ASC->HasMatchingGameplayTag(CuePoseAcc))
        {
            // Case 2c: uniform rotation in wrong pose, then Case 2b

            RemoveActiveGameplayEffect(TorqueHandle, *ActorInfo, ActivationInfo);
            co_await Latent::Seconds((DeltaThetaAbs - BreakingDistanceMedium) / OmegaAbs);
            DeltaTheta = NewDeltaTheta(MyPawn, LookAtAngle, Alpha, Omega, SignDeltaTheta * BreakingDistanceMedium);
        }
        else if(DeltaThetaAbs == BreakingDistanceLong && !ASC->HasMatchingGameplayTag(CuePoseAcc))
        {
            // Case 2d, at long breaking distance
            // idle pose, uniform rotation, then Case 2b

            RemoveActiveGameplayEffect(TorqueHandle, *ActorInfo, ActivationInfo);
            ASC->RemovePoseCue(Tag.CuePoseTorqueCCW);
            ASC->RemovePoseCue(Tag.CuePoseTorqueCW);
            co_await Latent::Seconds(TransitionTime);
            DeltaTheta = NewDeltaTheta(MyPawn, LookAtAngle, Alpha, Omega, SignDeltaTheta * BreakingDistanceMedium);
        }
        else
        /* implicit:
         * if( (BreakingDistanceMedium < DeltaThetaAbs < BreakingDistanceLong && ASC->HasMatchingGameplayTag(CuePosePositive)
         *   || DeltaThetaAbs > BreakingDistanceLong
         * )
        */
        {
            const float DeltaThetaAbsEffective = DeltaThetaAbs - (ASC->HasMatchingGameplayTag(CuePoseAcc) ? 0. : TransitionTime * OmegaAbs);
            const float Term = Alpha * TransitionTime / 2.;
            // the maximally possible speed that allows to reach the lookAt angle without overshoot
            const float OmegaBar =
                FMath::Min
                    ( OmegaMax
                    , FMath::Sqrt
                            (FMath::Pow(Term, 2)
                            + Alpha * DeltaThetaAbsEffective
                            + FMath::Pow(OmegaAbs, 2) / 2.
                            )
                      - Term
                    );

            if(OmegaAbs < OmegaBar)
            {
                // Case 4a: positive torque until OmegaBar, then either Case 4b or Case 2b

                const bool b1 = ASC->RemovePoseCue(CuePoseDec);
                const bool b2 = ASC->AddPoseCue(CuePoseAcc);
                if(b1 || b2)
                {
                    RemoveActiveGameplayEffect(TorqueHandle, *ActorInfo, ActivationInfo);
                    co_await Latent::Seconds(TransitionTime);
                }

                SetGameplayEffectTorque(Handle, ActorInfo, ActivationInfo, SpecTorqueAcc);
                co_await Latent::Seconds((OmegaBar - OmegaAbs) / Alpha);
                RemoveActiveGameplayEffect(TorqueHandle, *ActorInfo, ActivationInfo);

                MyPawn->SetOmega(SignDeltaTheta * OmegaBar);
                const float New = OmegaBar < OmegaMax
                    ? // goto Case 2b
                        SignDeltaTheta * (FMath::Pow(OmegaBar, 2) / 2. / Alpha + OmegaBar * TransitionTime)
                    : // goto Case 4b
                        UFunctionLib::WrapRadians(LookAtAngle - MyPawn->GetActorQuat().GetTwistAngle(FVector::UnitZ()));
                DeltaTheta = NewDeltaTheta(MyPawn, LookAtAngle, Alpha, SignDeltaTheta * OmegaBar, New);
            }
            else if(OmegaAbs == OmegaBar)
            {
                // Case 4b: uniform rotation at OmegaBar, then Case 2b
                
                RemoveActiveGameplayEffect(TorqueHandle, *ActorInfo, ActivationInfo);
                ASC->RemovePoseCue(Tag.CuePoseTorqueCCW);
                ASC->RemovePoseCue(Tag.CuePoseTorqueCW);
                co_await Latent::Seconds((DeltaThetaAbs - BreakingDistanceMedium) / OmegaAbs);
                DeltaTheta = NewDeltaTheta(MyPawn, LookAtAngle, Alpha, Omega, SignDeltaTheta * BreakingDistanceMedium);
            }
            else
            {
                UE_LOGFMT(LogMyGame, Error, "LookAt: stuck in endless loop. Payload: {P}, DeltaTheta: {D}, Omega: {O}, OmegaBar: {B}"
                    , Payload, DeltaTheta, Omega, OmegaBar);
                break;
            }
        }
    }
}

void UGA_LookAt::SetGameplayEffectTorque(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FGameplayEffectSpecHandle TorqueSpec)
{
    const auto* ASC = UMyAbilitySystemComponent::Get(ActorInfo);
    const auto* GE = ASC->GetActiveGameplayEffect(TorqueHandle);
    if(!GE || GE->Spec.Def != TorqueSpec.Data->Def)
    {
        if(GE)
            RemoveActiveGameplayEffect(TorqueHandle, *ActorInfo, ActivationInfo);
        TorqueHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, TorqueSpec);
    }
}

float UGA_LookAt::NewDeltaTheta(AMyPawn* MyPawn, float InLookAtAngle, float Alpha, float Omega, float InDeltaTheta) const
{
    const FQuat NewQuat = FQuat(FVector::UnitZ(), InLookAtAngle - InDeltaTheta);
    const FQuat OldQuat = MyPawn->GetActorQuat();
    const float Delta = (NewQuat * OldQuat.Inverse()).GetTwistAngle(FVector::UnitZ());
	if(FMath::Abs(Delta) > 0.05)
	    UE_LOGFMT(LogMyGame, Error, "Setting ActorRot to {NEW}, old ActorRot = {OLD}; Delta = {DELTA}, |Delta| > 0.05"
            , NewQuat.GetTwistAngle(FVector::UnitZ()), OldQuat.GetTwistAngle(FVector::UnitZ()), Delta);
    MyPawn->SetActorRotation(NewQuat);
    return
        FMath::Sign(Omega) * InDeltaTheta < pow(Omega, 2) / Alpha + 2 * TransitionTime * FMath::Abs(Omega) - PI
            ? InDeltaTheta + FMath::Sign(Omega) * TWO_PI
            : InDeltaTheta;
}
