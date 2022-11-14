#include "Actors/MyPawn.h"

#include "Modes/MyGameState.h"
#include "Net/UnrealNetwork.h"

AMyPawn::AMyPawn()
{
	PrimaryActorTick.bCanEverTick = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    Root->SetMobility(EComponentMobility::Stationary);
    SetRootComponent(Root);
    
    MovableRoot = CreateDefaultSubobject<USceneComponent>(TEXT("MovableRoot"));
    MovableRoot->SetupAttachment(Root);
    
    Orbit = CreateDefaultSubobject<UOrbitComponent>(TEXT("Orbit"));
    Orbit->SetupAttachment(Root);

    SplineMeshParent = CreateDefaultSubobject<USceneComponent>(TEXT("SplineMesh"));
    SplineMeshParent->SetupAttachment(Orbit);
    SplineMeshParent->SetMobility(EComponentMobility::Stationary);

	bNetLoadOnClient = false;
	bReplicates = true;
	AActor::SetReplicateMovement(false);
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
		, GetMovableRoot()->GetComponentLocation()
		, GetMovableRoot()->GetComponentLocation() + 1000. * GetMovableRoot()->GetForwardVector()
		, 20
		, FColor::Yellow
		);
	
	if(RP_bIsAccelerating)
	{
		const FPhysics Physics = UStateLib::GetPhysicsUnsafe(this);
		const FInstanceUI InstanceUI = UStateLib::GetInstanceUIUnsafe(this);
		const float DeltaV = AccelerationSI / Physics.ScaleFactor * DeltaSeconds;
		GetOrbit()->AddVelocity(GetMovableRoot()->GetForwardVector() * DeltaV, Physics, InstanceUI);
	}
}

void AMyPawn::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	IHasOrbit::Construction(Transform);
}

void AMyPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// two different ways to go about network lag and potential packet loss
	//
	// in case of rotation: "movement prediction": leaving the implementation to the client, allowing for quick movement;
	// in case this goes out of syn (COND_SkipOwner prevents re-sync), the player looks into the wrong direction for a while;
	//DOREPLIFETIME_CONDITION(APawnInSpace, RP_BodyRotation   , COND_SkipOwner)
	// just removing the COND_SkipOwner makes sure that the client's authority doesn't last more than a couple of frames
	DOREPLIFETIME(AMyPawn, RP_BodyRotation)

	// in case of acceleration: full server-control: the client won't react to the key press until the action has
	// round-tripped, i.e. there is no movement prediction
	DOREPLIFETIME(AMyPawn, RP_bIsAccelerating)
}
