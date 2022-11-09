#include "Actors/PawnInSpace.h"

#include "Modes/MyGameState.h"
#include "Net/UnrealNetwork.h"

APawnInSpace::APawnInSpace()
{
	PrimaryActorTick.bCanEverTick = false;

	// components

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	//Root->SetMobility(EComponentMobility::Stationary);
	SetRootComponent(Root);

	MovableRoot = CreateDefaultSubobject<USceneComponent>(TEXT("MovableRoot"));
	MovableRoot->SetupAttachment(Root);
	
	Orbit = CreateDefaultSubobject<UOrbitComponent>(TEXT("Orbit"));
	Orbit->SetupAttachment(Root);
	Orbit->SetMovableRoot(MovableRoot);
	
	SplineMeshParent = CreateDefaultSubobject<USceneComponent>(TEXT("SplineMesh"));
	SplineMeshParent->SetupAttachment(Orbit);
	SplineMeshParent->SetMobility(EComponentMobility::Stationary);
	Orbit->SetSplineMeshParent(SplineMeshParent);

	// for the editor
	Orbit->UpdateVisibility(UStateLib::GetInstanceUIEditorDefault());
	
	bNetLoadOnClient = false;
	AActor::SetReplicateMovement(false);
}

void APawnInSpace::UpdateLookTarget(FVector Target)
{
	// TODO
}

void APawnInSpace::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	DrawDebugDirectionalArrow
		( GetWorld()
		, MovableRoot->GetComponentLocation()
		, MovableRoot->GetComponentLocation() + 1000. * MovableRoot->GetForwardVector()
		, 20
		, FColor::Yellow
		);
	
	if(RP_bIsAccelerating)
	{
		const FPhysics Physics = UStateLib::GetPhysicsUnsafe(this);
		const FInstanceUI InstanceUI = UStateLib::GetInstanceUIUnsafe(this);
		const float DeltaV = AccelerationSI / Physics.ScaleFactor * DeltaSeconds;
		Orbit->AddVelocity(MovableRoot->GetForwardVector() * DeltaV, Physics, InstanceUI);
	}
}

#if WITH_EDITOR
void APawnInSpace::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Orbit->SetCircleOrbit
		( UStateLib::GetPhysicsEditorDefault()
		, UStateLib::GetInstanceUIEditorDefault()
		);
}
#endif 

void APawnInSpace::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// two different ways to go about network lag and potential packet loss
	//
	// in case of rotation: "movement prediction": leaving the implementation to the client, allowing for quick movement;
	// in case this goes out of syn (COND_SkipOwner prevents re-sync), the player looks into the wrong direction for a while;
	//DOREPLIFETIME_CONDITION(APawnInSpace, RP_BodyRotation   , COND_SkipOwner)
	// just removing the COND_SkipOwner makes sure that the client's authority doesn't last more than a couple of frames
	DOREPLIFETIME(APawnInSpace, RP_BodyRotation)

	// in case of acceleration: full server-control: the client won't react to the key press until the action has
	// round-tripped, i.e. there is no movement prediction
	DOREPLIFETIME(APawnInSpace, RP_bIsAccelerating)
}