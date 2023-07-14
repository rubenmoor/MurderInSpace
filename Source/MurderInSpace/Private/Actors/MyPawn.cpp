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

	// rotation angle
	const double Theta = Omega * DeltaSeconds;
	
	const FQuat MyQuat = GetActorQuat();
	const double Delta = RP_RotationAim.GetTwistAngle(FVector::UnitZ()) - MyQuat.GetTwistAngle(FVector::UnitZ());
	if(FMath::Abs(Delta) > Theta)
	{
		SetActorRotation(MyQuat * FQuat::MakeFromRotationVector(FVector::UnitZ() * Theta * FMath::Sign(Delta)));
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

	// in case of acceleration: full server-control: the client won't react to the key press until the action has
	// round-tripped, i.e. there is no movement prediction
	DOREPLIFETIME(AMyPawn, RP_bIsAccelerating)
	DOREPLIFETIME(AMyPawn, RP_bTowardsCircle)
}
