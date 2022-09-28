#include "PawnInSpace.h"

#include "CharacterInSpace.h"
#include "MyGameInstance.h"
#include "MyGameState.h"
#include "Actions/PawnAction.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"

APawnInSpace::APawnInSpace()
{
	PrimaryActorTick.bCanEverTick = false;

	// components

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Root->SetMobility(EComponentMobility::Stationary);
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
}

void APawnInSpace::UpdateLookTarget(FVector Target)
{
	// TODO
}

void APawnInSpace::LookAt(FVector VecP)
{
	const FVector VecMe = MovableRoot->GetComponentLocation();
	const FVector VecDirection = VecP - VecMe;
	const FQuat Quat = FQuat::FindBetween(FVector(0, 1, 0), VecDirection);
	const float AngleDelta =
		Quat.GetTwistAngle(FVector(0, 0, 1)) - MovableRoot->GetComponentQuat().GetTwistAngle(FVector(0, 0, 1));
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

	const TObjectPtr<AMyGameState> GS = GetWorld()->GetGameState<AMyGameState>();
	FSpaceParams SP = GS->GetSpaceParams();
	if(bIsAccelerating)
	{
		Orbit->AddVelocity(GetActorForwardVector() * AccelerationSI / SP.ScaleFactor * DeltaSeconds, GS);
	}
}

void APawnInSpace::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Orbit->InitializeCircle(MovableRoot->GetComponentLocation(), AMyGameState::DefaultSpaceParams);
}
