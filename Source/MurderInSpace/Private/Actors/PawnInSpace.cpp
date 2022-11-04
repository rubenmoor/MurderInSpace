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
	Orbit->UpdateVisibility(UStateLib::GetPlayerUIEditorDefault());
	
	AActor::SetReplicateMovement(false);
}

void APawnInSpace::UpdateLookTarget(FVector Target)
{
	// TODO
}

void APawnInSpace::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	MovableRoot->SetWorldRotation(BodyRotation);
	
	DrawDebugDirectionalArrow
		( GetWorld()
		, MovableRoot->GetComponentLocation()
		, MovableRoot->GetComponentLocation() + 1000. * MovableRoot->GetForwardVector()
		, 20
		, FColor::Yellow
		);
	
	if(bIsAccelerating)
	{
		const FPhysics Physics = UStateLib::GetPhysicsUnsafe(this);
		const FPlayerUI PlayerUI = UStateLib::GetPlayerUIUnsafe
			(this
			, FLocalPlayerContext(GetGameInstance()->GetPrimaryPlayerController())
			);
		const float DeltaV = AccelerationSI / Physics.ScaleFactor * DeltaSeconds;
		Orbit->AddVelocity(MovableRoot->GetForwardVector() * DeltaV, Physics, PlayerUI);
	}
}

#if WITH_EDITOR
void APawnInSpace::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Orbit->SetCircleOrbit
		( MovableRoot->GetComponentLocation()
		, UStateLib::GetPhysicsEditorDefault()
		, UStateLib::GetPlayerUIEditorDefault()
		);
}
#endif 

void APawnInSpace::BeginPlay()
{
	Super::BeginPlay();
	Orbit->SetCircleOrbit
		( MovableRoot->GetComponentLocation()
		, UStateLib::GetPhysicsUnsafe(this)
		, UStateLib::GetPlayerUIEditorDefault()
		);
}


void APawnInSpace::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APawnInSpace, BodyRotation);
}