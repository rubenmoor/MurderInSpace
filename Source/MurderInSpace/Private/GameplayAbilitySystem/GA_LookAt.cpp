#include "GameplayAbilitySystem/GA_LookAt.h"

#include <format>

#include "GameplayAbilitySystem/MyAttributes.h"
#include "MyGameplayTags.h"
#include "GameplayAbilitySystem/GE_TorqueCCW.h"
#include "GameplayAbilitySystem/GE_TorqueCW.h"
#include "GameplayAbilitySystem/MyAbilitySystemComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Lib/FunctionLib.h"
#include "Logging/StructuredLog.h"
#include "Modes/MyGameInstance.h"
#include "UE5Coro/LatentAwaiters.h"

using namespace UE5Coro;
using namespace UE5Coro::GAS;

UGA_LookAt::UGA_LookAt()
{
    const auto& Tag = FMyGameplayTags::Get();

    AbilityTags.AddTag(Tag.AbilityLookAt);
    
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
    
    UE_LOGFMT(LogMyGame, Display, "ExecuteAbility LookAt");
    
    if(!CommitAbility(Handle, ActorInfo, ActivationInfo))
        co_await Latent::Cancel();
    
    if(auto* OnBlockingAbilityEnded = TurnBlocked(Handle, ActorInfo))
    {
        UE_LOGFMT(LogMyGame, Display, "{NAME} waiting turn", GetFName());
        co_await Latent::UntilDelegate(*OnBlockingAbilityEnded);
        UE_LOGFMT(LogMyGame, Display, "{NAME} started after waiting turn", GetFName());
    }
    
    auto* ASC = UMyAbilitySystemComponent::Get(ActorInfo);
    ASC->AbilityAwaitingTurn = FGameplayAbilitySpecHandle();
    
    const float OmegaMaxByAttribute = ASC->GetNumericAttribute(UAttrSetAcceleration::GetOmegaMaxAttribute());
    const float Alpha = ASC->GetNumericAttribute(UAttrSetAcceleration::GetTorqueMaxAttribute());
    
    auto* MyPawn = Cast<AMyPawn>(ActorInfo->OwnerActor);
    const auto& Tag = FMyGameplayTags::Get();
    
    const float LookAtAngle =
        UFunctionLib::WrapRadians
            (MyPawn->GetActorQuat().GetTwistAngle(FVector::UnitZ()) + TriggerEventData->EventMagnitude);

    float DeltaTheta = NewDeltaTheta(MyPawn, LookAtAngle, Alpha, MyPawn->GetOmega(), TriggerEventData->EventMagnitude);
    
    while(true)
    {
        const float Omega = MyPawn->GetOmega();
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

        UE_LOGFMT
            ( LogMyGame
            , Display
            , "Omega: {OMEGA}, %Max {OMEGAREL}%, DeltaTheta: {DeltaTheta}, BreakingDistanceShort: {BREAKING}, Pose: {POSE}"
            , Omega
            , FMath::RoundToInt32(OmegaAbs / OmegaMax * 100.)
            , DeltaTheta
            , BreakingDistanceShort
            , ASC->HasMatchingGameplayTag(CuePoseDec) ? "Dec" : ASC->HasMatchingGameplayTag(CuePoseAcc) ? "Acc" : "Idle"
            );
        if(OmegaAbs > OmegaMax)
        {
            UE_LOGFMT(LogMyGame, Error, "Omega: {OMEGA}, OmegaMax: {OMEGAMAX}, |Omega| > OmegaMax"
                , Omega, OmegaMax);
            break;
        }
        if(Omega == 0. && DeltaTheta == 0.)
        {
            break;
        }
        else if(Omega * DeltaTheta < 0. || DeltaThetaAbs < BreakingDistanceShort || (DeltaThetaAbs < BreakingDistanceMedium && !ASC->HasMatchingGameplayTag(CuePoseDec)))
        {
            UE_LOGFMT(LogMyGame, Display, "Case 5");
            
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
            UE_LOGFMT(LogMyGame, Display, "Case 1");
            // Case 1: at short breaking distance, already in negative pose: decelerate until stop
            
            SetGameplayEffectTorque(Handle, ActorInfo, ActivationInfo, SpecTorqueDec);
            co_await Latent::Seconds(OmegaAbs / Alpha);
            RemoveActiveGameplayEffect(TorqueHandle, *ActorInfo, ActivationInfo);

            MyPawn->SetOmega(0.);
            DeltaTheta = NewDeltaTheta(MyPawn, LookAtAngle, Alpha, 0., 0.);

            ASC->RemovePoseCue(CuePoseDec);
            co_await Latent::Seconds(TransitionTime);
        }
        else if(DeltaThetaAbs < BreakingDistanceMedium && ASC->HasMatchingGameplayTag(CuePoseDec))
        {
            UE_LOGFMT(LogMyGame, Display, "Case 2a");
            // Case 2a: uniform rotation in correct pose, then Case 1
            
            RemoveActiveGameplayEffect(TorqueHandle, *ActorInfo, ActivationInfo);
            const float DeltaTime = (DeltaThetaAbs - BreakingDistanceShort) / OmegaAbs;
            co_await Latent::Seconds(DeltaTime);
            UE_LOGFMT(LogMyGame, Display, "Case 2a: Uniform rotation on correct pose: Time: {TIME}", DeltaTime);
            DeltaTheta = NewDeltaTheta(MyPawn, LookAtAngle, Alpha, Omega, SignDeltaTheta * BreakingDistanceShort);
        }
        else if(DeltaThetaAbs == BreakingDistanceMedium)
        {
            UE_LOGFMT(LogMyGame, Display, "Case 2b");
            // Case 2b: at medium breaking distance: uniform rotation during pose change and then Case 1
            
            RemoveActiveGameplayEffect(TorqueHandle, *ActorInfo, ActivationInfo);
            ASC->RemovePoseCue(CuePoseAcc);
            ASC->AddPoseCue(CuePoseDec);
            co_await Latent::Seconds(TransitionTime);
            DeltaTheta = NewDeltaTheta(MyPawn, LookAtAngle, Alpha, Omega, SignDeltaTheta * BreakingDistanceShort);
        }
        else if(DeltaThetaAbs < BreakingDistanceLong && !ASC->HasMatchingGameplayTag(CuePoseAcc))
        {
            UE_LOGFMT(LogMyGame, Display, "Case 2c");
            // Case 2c: uniform rotation in wrong pose, then Case 2b

            RemoveActiveGameplayEffect(TorqueHandle, *ActorInfo, ActivationInfo);
            co_await Latent::Seconds((DeltaThetaAbs - BreakingDistanceMedium) / OmegaAbs);
            DeltaTheta = NewDeltaTheta(MyPawn, LookAtAngle, Alpha, Omega, SignDeltaTheta * BreakingDistanceMedium);
        }
        else if(DeltaThetaAbs == BreakingDistanceLong && !ASC->HasMatchingGameplayTag(CuePoseAcc))
        {
            UE_LOGFMT(LogMyGame, Display, "Case 2d");
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

            UE_LOGFMT(LogMyGame, Display, "OmegaBar: {OMEGABAR}, OmegaMax: {OMEGAMAX}, OmegaBar/OmegaMax: {REL}%"
                , OmegaBar, OmegaMax, FMath::RoundToInt32(OmegaBar / OmegaMax * 100.));

            if(OmegaAbs < OmegaBar)
            {
                UE_LOGFMT(LogMyGame, Display, "Case 4a");
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
                float New;
                if(OmegaBar < OmegaMax)
                {
                    // goto Case 2b
                    New = SignDeltaTheta * (FMath::Pow(OmegaBar, 2) / 2. / Alpha + OmegaBar * TransitionTime);
                    UE_LOGFMT(LogMyGame, Display, "Case 4a.1: New DeltaTheta set for Case 2b");
                }
                else
                {
                    // goto Case 4b
                    New = UFunctionLib::WrapRadians(LookAtAngle - MyPawn->GetActorQuat().GetTwistAngle(FVector::UnitZ()));
                }
                DeltaTheta = NewDeltaTheta(MyPawn, LookAtAngle, Alpha, SignDeltaTheta * OmegaBar, New);
            }
            else if(OmegaAbs == OmegaBar)
            {
                UE_LOGFMT(LogMyGame, Display, "Case 4b");
                // Case 4b: uniform rotation at OmegaBar, then Case 2b
                
                RemoveActiveGameplayEffect(TorqueHandle, *ActorInfo, ActivationInfo);
                ASC->RemovePoseCue(Tag.CuePoseTorqueCCW);
                ASC->RemovePoseCue(Tag.CuePoseTorqueCW);
                co_await Latent::Seconds((DeltaThetaAbs - BreakingDistanceMedium) / OmegaAbs);
                DeltaTheta = NewDeltaTheta(MyPawn, LookAtAngle, Alpha, Omega, SignDeltaTheta * BreakingDistanceMedium);
            }
        }
    }
}

void UGA_LookAt::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
    UE_LOGFMT(LogMyGame, Warning, "LookAt cancelled");
    //RemoveActiveGameplayEffect(TorqueHandle, *ActorInfo, ActivationInfo);
    Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
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
    UE_LOGFMT(LogMyGame, Display, "NewDeltaTheta: InLookAtAngle: {INLOOKATANGLE}, Omega: {OMEGA}, InDeltaTheta: {INDELTATHETA}"
        , InLookAtAngle, Omega, InDeltaTheta);
    const FQuat NewQuat = FQuat(FVector::UnitZ(), InLookAtAngle - InDeltaTheta);
    const FQuat OldQuat = MyPawn->GetActorQuat();
    const float Delta = (NewQuat * OldQuat.Inverse()).GetTwistAngle(FVector::UnitZ());
	if(FMath::Abs(Delta) > 0.05)
	{
	    UE_LOGFMT(LogMyGame, Error, "Setting ActorRot to {NEW}, old ActorRot = {OLD}; Delta = {DELTA}, |Delta| > 0.05"
            , NewQuat.GetTwistAngle(FVector::UnitZ()), OldQuat.GetTwistAngle(FVector::UnitZ()), Delta);
	    //UKismetSystemLibrary::QuitGame(this, GetWorld()->GetFirstPlayerController(), EQuitPreference::Quit, false);
	}
	else
		UE_LOGFMT(LogMyGame, Warning, "Setting ActorRot to {NEW}, old ActorRot = {OLD}; Delta = {DELTA}"
			, NewQuat.GetTwistAngle(FVector::UnitZ()), OldQuat.GetTwistAngle(FVector::UnitZ()), Delta);
    MyPawn->SetActorRotation(NewQuat);
    return
        FMath::Sign(Omega) * InDeltaTheta < pow(Omega, 2) / Alpha + 2 * TransitionTime * FMath::Abs(Omega) - PI
            ? InDeltaTheta + FMath::Sign(Omega) * TWO_PI
            : InDeltaTheta;
}
