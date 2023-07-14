#include "Actors/MyPawn.h"

#include "Actors/MyCharacter.h"
#include "HUD/MyHUD.h"
#include "Modes/MyGameInstance.h"
#include "Modes/MyGameState.h"
#include "Modes/MyPlayerController.h"
#include "Net/UnrealNetwork.h"

AMyPawn::AMyPawn()
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
}

void AMyPawn::UpdateLookTarget(FVector Target)
{
	// TODO
}

void AMyPawn::SetRotationAim(const FQuat& Quat)
{
	RP_RotationAim = Quat;
}

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
	const FPhysics Physics = MyState->GetPhysics(GS);
	const auto* GI = GetGameInstance<UMyGameInstance>();
	const FInstanceUI InstanceUI = MyState->GetInstanceUI(GI);
	
	if(RP_bIsAccelerating)
	{
		const double DeltaV = AccelerationSI / FPhysics::LengthScaleFactor * DeltaSeconds;
		RP_Orbit->Update(GetActorForwardVector() * DeltaV, Physics, InstanceUI);
	}
	else if(RP_bTowardsCircle)
	{
		const auto VecVCircle = RP_Orbit->GetCircleVelocity(Physics);
		const auto VecTarget = std::copysign(1., RP_Orbit->GetVecVelocity().Dot(VecVCircle)) * VecVCircle;
		RP_RotationAim = FQuat::FindBetween(FVector(1., 0., 0.), VecTarget);
		SetActorRotation(RP_RotationAim);
		const FVector VecDelta = VecTarget - RP_Orbit->GetVecVelocity();
		const double DeltaV = AccelerationSI / FPhysics::LengthScaleFactor * DeltaSeconds;
		if(VecDelta.Length() > DeltaV)
		{
			RP_Orbit->Update(VecDelta.GetSafeNormal() * DeltaV, Physics, InstanceUI);
		}
	}

	// rotating towards `RP_RotationAim` at speed `Omega`

	const FQuat MyQuat = GetActorQuat();
	const double RemainingTheta = (RP_RotationAim * MyQuat.Inverse()).GetTwistAngle(FVector::UnitZ());

	const double BreakingDistance = FMath::Pow(Omega, 2) / 2. / RP_AlphaMax;
	if(Omega == 0. && FMath::Abs(RemainingTheta) > 1. * PI / 180)
	{
		// start acceleration in the direction of `RP_RotationAim`
		Alpha = FMath::Sign(RemainingTheta) * RP_AlphaMax;
	}
	else
	{
		const double NewOmega = Omega + Alpha * DeltaSeconds;

		if(RemainingTheta * Omega < 0.)
		{
			// moving away from `RotationAim`? turn around!
			Alpha = FMath::Sign(RemainingTheta) * RP_AlphaMax;
		}
		else if(FMath::Abs(RemainingTheta) <= BreakingDistance)
		{
			// decelarate when approaching `RotationAim`
			Alpha = -FMath::Sign(Omega) * RP_AlphaMax;
		}
		else if(Alpha != 0.)
		{
			if(FMath::Abs(NewOmega) >= RP_OmegaMax)
			{
				// reached maximum angular velocity
				Alpha = 0.;
				Omega = FMath::Sign(Omega) * RP_OmegaMax;
			}
			else if(NewOmega * Omega < 0.)
			{
				// change of sign => (over-)reached zero angular velocity
				Alpha = 0.;
				Omega = 0.;
			}
		}
	}
	Omega += Alpha * DeltaSeconds;
	const double DeltaTheta = Omega * DeltaSeconds;
	SetActorRotation(MyQuat * FQuat::MakeFromRotationVector(FVector::UnitZ() * DeltaTheta));

	if(++NTicks % 20 == 0)
	{
	UE_LOG(LogMyGame, Display, TEXT("DeltaTheta: %f, Omega: %f, Alpha: %f, Remaining Theta: %f, BreakingDistance: %f")
		, DeltaTheta * 180. / PI
		, Omega * 180. / PI
		, Alpha * 180. / PI
		, RemainingTheta * 180. / PI
		, BreakingDistance * 180. / PI
		)
	}
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
	RP_RotationAim = GetActorQuat();
}

void AMyPawn::BeginPlay()
{
	Super::BeginPlay();
	
    if(GetLocalRole() == ROLE_Authority)
    {
        SetActorTickEnabled(true);
    }
    else
    {
        SetReadyFlags(EMyPawnReady::InternalReady);
    }
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
			RP_Orbit->Update(PhysicsEditorDefault, InstanceUIEditorDefault);
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
		SetActorTickEnabled(true);
	}
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
	DOREPLIFETIME(AMyPawn, RP_RotationAim)
	DOREPLIFETIME(AMyPawn, RP_AlphaMax)
	DOREPLIFETIME(AMyPawn, RP_OmegaMax)

	// in case of acceleration: full server-control: the client won't react to the key press until the action has
	// round-tripped, i.e. there is no movement prediction
	DOREPLIFETIME(AMyPawn, RP_bIsAccelerating)
	DOREPLIFETIME(AMyPawn, RP_bTowardsCircle)
}
