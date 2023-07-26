#include "Actors/MyPawn.h"

#include "Actors/MyCharacter.h"
#include "GameplayAbilitySystem/MyAttributes.h"
#include "GameplayAbilitySystem/MyAbilitySystemComponent.h"
#include "GameplayAbilitySystem/MyGameplayAbility.h"
#include "MyGameplayTags.h"
#include "HUD/MyHUD.h"
#include "Modes/MyGameInstance.h"
#include "Modes/MyGameState.h"
#include "Modes/MyPlayerController.h"
#include "Net/UnrealNetwork.h"

AMyPawn::AMyPawn(): APawn()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	bNetLoadOnClient = false;
	bReplicates = true;
	// TODO: not sure if necessary, but not harmful either
	bAlwaysRelevant = true;
	AActor::SetReplicateMovement(false);

	Root = CreateDefaultSubobject<USceneComponent>("Root");
	SetRootComponent(Root);

	AbilitySystemComponent = CreateDefaultSubobject<UMyAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->ReplicationMode = EGameplayEffectReplicationMode::Mixed;

	AttrSetAcceleration = CreateDefaultSubobject<UAttrSetAcceleration>("AttributeSetTorque");
}

void AMyPawn::UpdateLookTarget(FVector Target)
{
	// TODO
}

// void AMyPawn::SetRotationAim(const FQuat& InQuat)
// {
// 	RP_QuatRotationAim = InQuat;
// 	const FQuat MyQuat = GetActorQuat();
// 	const double RemainingTheta = (RP_QuatRotationAim * MyQuat.Inverse()).GetTwistAngle(FVector::UnitZ());
// 	const auto Tag = FMyGameplayTags::Get();
// 	// initiate rotation or adjust direction
// 	if(Omega == 0. || RemainingTheta * Omega < 0.)
// 	{
// 		// start acceleration in the direction of `RP_RotationAim`
// 		FGameplayTag TorqueTag;
// 		if(RemainingTheta > 0.)
// 		{
// 			TorqueTag = Tag.HasTorqueCCW;
// 		}
// 		else
// 		{
// 			TorqueTag = Tag.HasTorqueCW;
// 		}
// 		AbilitySystemComponent->TryActivateAbilitiesByTag(TorqueTag.GetSingleTagContainer());
// 	}
// }

void AMyPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	DrawDebugDirectionalArrow
		( GetWorld()
		, GetActorLocation()
		, GetActorLocation() + 1000. * GetActorForwardVector()
		, 20
		, FColor::Yellow
		);
	
	UMyState* MyState = GEngine->GetEngineSubsystem<UMyState>();
	const UWorld* World = GetWorld();
	const auto* GS = World->GetGameState<AMyGameState>();
	const FPhysics Physics = GS->RP_Physics;
	const auto* GI = GetGameInstance<UMyGameInstance>();
	const auto Tag = FMyGameplayTags::Get();

	if(AbilitySystemComponent->HasMatchingGameplayTag(Tag.AccelerationTranslational))
	{
		const float AccelerationSI = AttrSetAcceleration->GetAccelerationSIMax();
		const double DeltaV = AccelerationSI / FPhysics::LengthScaleFactor * DeltaSeconds;
		RP_Orbit->Update(GetActorForwardVector() * DeltaV, Physics);
	}
	else if(AbilitySystemComponent->HasMatchingGameplayTag(Tag.AbilityMoveTowardsCircle))
	{
		const float AccelerationSI = AttrSetAcceleration->AccelerationSIMax.GetCurrentValue();
		const auto VecVCircle = RP_Orbit->GetCircleVelocity(Physics);
		const auto VecTarget = std::copysign(1., RP_Orbit->GetVecVelocity().Dot(VecVCircle)) * VecVCircle;
		RP_QuatRotationAim = FQuat::FindBetween(FVector(1., 0., 0.), VecTarget);
		SetActorRotation(RP_QuatRotationAim);
		const FVector VecDelta = VecTarget - RP_Orbit->GetVecVelocity();
		const double DeltaV = AccelerationSI / FPhysics::LengthScaleFactor * DeltaSeconds;
		if(VecDelta.Length() > DeltaV)
		{
			RP_Orbit->Update(VecDelta.GetSafeNormal() * DeltaV, Physics);
		}
	}

	// rotating towards `RP_RotationAim` at speed `Omega`

	// const float TorqueMax = AttrSetAcceleration->TorqueMax.GetCurrentValue();
	// float Alpha;
	// const auto MaybeTorqueTag = AbilitySystemComponent->FindTag(Tag.HasTorque);
	// if(MaybeTorqueTag == Tag.HasTorqueCCW)
	// {
	// 	Alpha = TorqueMax;
	// }
	// else if(MaybeTorqueTag == Tag.HasTorqueCW)
	// {
	// 	Alpha = -TorqueMax;
	// }
	// else
	// {
	// 	Alpha = 0.;
	// }
	// double NewOmega = Omega + Alpha * DeltaSeconds;
	//const FQuat MyQuat = GetActorQuat();
	//const double RemainingTheta = (RP_QuatRotationAim * MyQuat.Inverse()).GetTwistAngle(FVector::UnitZ());

	//const double BreakingDistance = FMath::Pow(Omega, 2) / 2. / TorqueMax;
	//if(Omega != 0.) 
	//{
	//	if(FMath::Abs(RemainingTheta) <= BreakingDistance)
	//	{
	//		// decelarate when approaching `RotationAim`
	//		FGameplayTag TorqueTag;
	//		if(Omega < 0.)
	//		{
	//			TorqueTag = Tag.HasTorqueCCW;
	//		}
	//		else
	//		{
	//			TorqueTag = Tag.HasTorqueCW;
	//		}
	//		AbilitySystemComponent->TryActivateAbilitiesByTag(TorqueTag.GetSingleTagContainer());
	//	}
	//	else if(AbilitySystemComponent->FindTag(Tag.HasTorque).IsValid())
	//	{
	//		const float OmegaMax = AttrSetAcceleration->OmegaMax.GetCurrentValue();
	//		if(FMath::Abs(NewOmega) >= OmegaMax)
	//		{
	//			// reached maximum angular velocity
	//			AbilitySystemComponent->CancelAbilities(&Tag.HasTorque.GetSingleTagContainer());
	//			NewOmega = FMath::Sign(Omega) * OmegaMax;
	//		}
	//		else if(NewOmega * Omega < 0.)
	//		{
	//			// change of sign => (over-)reached zero angular velocity
	//			AbilitySystemComponent->CancelAbilities(&Tag.HasTorque.GetSingleTagContainer());
	//			NewOmega = 0.;
	//		}
	//	}
	//}
	//Omega = NewOmega;
	Omega += AttrSetAcceleration->GetTorque() * DeltaSeconds;
	const double DeltaTheta = Omega * DeltaSeconds;
	SetActorRotation(GetActorQuat() * FQuat::MakeFromRotationVector(FVector::UnitZ() * DeltaTheta));
}

void AMyPawn::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	if  (
		// Only the server spawns orbits
		   GetLocalRole()        == ROLE_Authority
    	   
		// avoid orbit spawning when editing and compiling blueprint
		&& GetWorld()->WorldType != EWorldType::EditorPreview
		
		// avoid orbit spawning when dragging an actor with orbit into the viewport at first
		// The preview actor that is created doesn't have a valid location
		// Once the actor is placed inside the viewport, it's no longer transient and the orbit is reconstructed properly
		// according to the actor location
		&& (!HasAnyFlags(RF_Transient) || Cast<AMyCharacter>(this))
		)
	{
		OrbitSetup(this);
	}
	RP_QuatRotationAim = GetActorQuat();
}

void AMyPawn::BeginPlay()
{
	Super::BeginPlay();
	
	for(const auto Ability : Abilities)
	{
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability));
	}
	
    if(GetLocalRole() == ROLE_Authority)
    {
		Initialize();
    }
    else
    {
        SetReadyFlags(EMyPawnReady::InternalReady);
    }
}

void AMyPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

#if WITH_EDITOR
void AMyPawn::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

    const FName Name = PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName();

    static const FName FNameOrbitColor = GET_MEMBER_NAME_CHECKED(AMyPawn, OrbitColor);

	if(Name == FNameOrbitColor)
	{
		if(IsValid(RP_Orbit))
		{
			RP_Orbit->Update(FPhysics());
		}
	}
}
#endif

void AMyPawn::Destroyed()
{
	Super::Destroyed();
	if(IsValid(RP_Orbit))
	{
		RP_Orbit->Destroy();
	}
}

void AMyPawn::OnRep_Orbit()
{
	if(GetLocalRole() == ROLE_AutonomousProxy)
	{
		GetController<AMyPlayerController>()->GetHUD<AMyHUD>()->SetReadyFlags(EHUDReady::PawnOrbitReady);
		SetReadyFlags(EMyPawnReady::OrbitReady);
	}
}

void AMyPawn::SetReadyFlags(EMyPawnReady ReadyFlags)
{
	MyPawnReady |= ReadyFlags;
	if(MyPawnReady == EMyPawnReady::All)
	{
		Initialize();
	}
}

void AMyPawn::Initialize()
{
	SetActorTickEnabled(true);
}

void AMyPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(AMyPawn, RP_Orbit, COND_InitialOnly)
	
	// two different ways to go about network lag and potential packet loss
	//
	// in case of rotation: "movement prediction": leaving the implementation to the client, allowing for quick movement;
	// in case this goes out of syn (COND_SkipOwner prevents re-sync), the player looks into the wrong direction for a while;
	//DOREPLIFETIME_CONDITION(APawnInSpace, RP_BodyRotation   , COND_SkipOwner)
	// just removing the COND_SkipOwner makes sure that the client's authority doesn't last more than a couple of frames
	DOREPLIFETIME(AMyPawn, RP_QuatRotationAim)
}
