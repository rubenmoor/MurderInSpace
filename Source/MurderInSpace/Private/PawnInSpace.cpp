#include "PawnInSpace.h"

#include "MyGameInstance.h"
#include "OrbitDataComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"

APawnInSpace::APawnInSpace()
{
	PrimaryActorTick.bCanEverTick = false;

	// components

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void APawnInSpace::UpdateLookTarget(FVector Target)
{
	// TODO
}

void APawnInSpace::LookAt(FVector VecP)
{
	const FVector VecMe = GetActorLocation();
	const FVector VecDirection = VecP - VecMe;
	const auto Quat = FQuat::FindBetween(FVector(1, 0, 0), VecDirection);
	const float AngleDelta = Quat.GetTwistAngle(FVector(0, 0, 1)) - GetActorQuat().GetTwistAngle(FVector(0, 0, 1));
	if(abs(AngleDelta) > 15. / 180. * PI)
	{
		SetActorRotation(Quat);
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
	
}
