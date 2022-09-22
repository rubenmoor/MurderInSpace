#include "PawnInSpace.h"

#include "CharacterInSpace.h"
#include "MyGameInstance.h"
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

	Orbit = CreateDefaultSubobject<UOrbitComponent>(TEXT("Orbit"));
	Orbit->SetupAttachment(Root);
	
	MovableRoot = CreateDefaultSubobject<USceneComponent>(TEXT("MovableRoot"));
	MovableRoot->SetupAttachment(Root);
}

void APawnInSpace::UpdateLookTarget(FVector Target)
{
	// TODO
}

void APawnInSpace::LookAt(FVector VecP)
{
	const FVector VecMe = MovableRoot->GetComponentLocation();
	const FVector VecDirection = VecP - VecMe;
	const auto Quat = FQuat::FindBetween(FVector(1, 0, 0), VecDirection);
	const float AngleDelta = Quat.GetTwistAngle(FVector(0, 0, 1)) - GetActorQuat().GetTwistAngle(FVector(0, 0, 1));
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

	const auto GI = GetGameInstance<UMyGameInstance>();
	const auto Alpha = GI->Alpha;
	const auto WorldRadius = GI->WorldRadius;
	const auto VecF1 = GI->VecF1;
	
	if(bIsAccelerating)
	{
		Orbit->AddVelocity(GetActorForwardVector() * AccelerationSI / GI->ScaleFactor * DeltaSeconds, Alpha, WorldRadius, VecF1);
	}

	const FVector NewLocation = Orbit->GetNextLocation(DeltaSeconds);
	MovableRoot->SetWorldLocation(NewLocation);
}

void APawnInSpace::BeginPlay()
{
	Super::BeginPlay();
	const UMyGameInstance* GI = GetWorld()->GetGameInstance<UMyGameInstance>();
	Orbit->InitializeCircle(GI->Alpha, GI->WorldRadius, GI->VecF1, MovableRoot->GetComponentLocation());
}
