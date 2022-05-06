#include "PawnInSpace.h"

#include "MyGameInstance.h"
#include "OrbitDataComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"

APawnInSpace::APawnInSpace()
{
	PrimaryActorTick.bCanEverTick = false;

	// components

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(Root);

	OrbitData = CreateDefaultSubobject<UOrbitDataComponent>(TEXT("Orbit Data"));
	OrbitData->RegisterComponent();
}

void APawnInSpace::UpdateLookTarget(FVector Target)
{
	// TODO
}

void APawnInSpace::LookAt(FVector VecP)
{
	const auto VecMe = GetActorLocation();
	const auto VecDirection = VecP - VecMe;
	const auto Quat = FQuat::FindBetween(FVector(1, 0, 0), VecDirection);
	const auto AngleDelta = Quat.GetAngle() - GetActorQuat().GetAngle();
	DrawDebugDirectionalArrow(GetWorld(), VecMe, VecP, 20, FColor::Red);
	if(abs(AngleDelta) > 15. / 180 * PI)
	{
		SetActorRotation(Quat);
	}
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
		OrbitData->AddVelocity(GetActorForwardVector() * AccelerationSI / GI->ScaleFactor * DeltaSeconds, Alpha, VecF1);
		OrbitData->Orbit->Update(Alpha, WorldRadius, VecF1);
	}
	
}
