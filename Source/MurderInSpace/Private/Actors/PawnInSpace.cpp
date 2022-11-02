#include "Actors/PawnInSpace.h"

#include "Modes/MyGameState.h"

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

void APawnInSpace::LookAt(FVector VecP)
{
	const FVector VecMe = MovableRoot->GetComponentLocation();
	const FVector VecDirection = VecP - VecMe;
	const FQuat Quat = FQuat::FindBetween(FVector(1, 0, 0), VecDirection);
	const float AngleDelta = Quat.GetTwistAngle
		( FVector(0, 0, 1)) -
			MovableRoot->GetComponentQuat().GetTwistAngle(FVector(0, 0, 1)
		);
	if(abs(AngleDelta) > 15. / 180. * PI)
	{
		MovableRoot->SetWorldRotation(Quat);
	}
	// debugging direction
	DrawDebugDirectionalArrow(GetWorld(), VecMe, VecP, 20, FColor::Red);
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

void APawnInSpace::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Orbit->InitializeCircle
		( MovableRoot->GetComponentLocation()
		, UStateLib::GetPhysicsEditorDefault()
		, UStateLib::GetPlayerUIEditorDefault()
		);
}
