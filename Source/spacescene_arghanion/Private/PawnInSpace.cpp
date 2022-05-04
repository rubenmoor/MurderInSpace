#include "PawnInSpace.h"

#include "MyGameInstance.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"

APawnInSpace::APawnInSpace()
{
	PrimaryActorTick.bCanEverTick = true;

	// components

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(Root);

	// if(Orbit)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("APawnInSpace::APawnInSpace: Orbit not null"))
	// }
	// else
	// {
	// 	if(const auto World = GetWorld())
	// 	{
	// 		if(OrbitClass)
	// 		{
	// 			Orbit = World->SpawnActor<AOrbit>(OrbitClass);
	// 			if(Orbit)
	// 			{
	// 				Orbit->Initialize(FVector::Zero(), this);
	// 			}
	// 		}
	// 		else
	// 		{
	// 			UE_LOG(LogTemp, Warning, TEXT("APawnInSpace::APawnInSpace: OrbitClass null"))
	// 		}
	// 	}
	// 	else
	// 	{
	// 		UE_LOG(LogTemp, Warning, TEXT("APawnInSpace::APawnInSpace: World null"))
	// 	}
	// }
}

void APawnInSpace::UpdateMU(float MU, float RMAX) const
{
	Orbit->UpdateOrbit(MU, RMAX);
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

	if(bIsAccelerating)
	{
		const auto GI = GetGameInstance<UMyGameInstance>();
		const auto VecV = Orbit->GetVecVelocity();
		const auto NewVecV = VecV + GetActorForwardVector() * AccelerationSI / GI->ScaleFactor * DeltaSeconds;
		Orbit->UpdateOrbit(NewVecV, GI->Alpha, GI->WorldRadius);
	}
}

void APawnInSpace::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	static const FName FNameOrbit = GET_MEMBER_NAME_CHECKED(APawnInSpace, Orbit);
	const auto Name = PropertyChangedEvent.GetPropertyName();
	
	if(Name == FNameOrbit)
	{
		const auto VecF1 = UMyGameInstance::EditorDefaultVecF1;
		const auto Alpha = UMyGameInstance::EditorDefaultAlpha;
		const auto VecR = GetActorLocation() - VecF1;
		const auto VecV = FVector(0, 0, 1).Cross(VecR).GetSafeNormal() * sqrt(Alpha / VecR.Length());
		Orbit->SetupActorInSpace(this, UMyGameInstance::EditorDefaultVecF1, VecV);
	}
	
	if(Orbit)
	{
		Orbit->UpdateOrbit(UMyGameInstance::EditorDefaultAlpha, UMyGameInstance::EditorDefaultWorldRadiusUU);
	}
	
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void APawnInSpace::SpawnOrbit()
{
 	if(Orbit)
 	{
 		UE_LOG(LogTemp, Warning, TEXT("APawnInSpace::SpawnOrbit: Orbit already set; skipping"))
 	}
    else
    {
 		Orbit = GetWorld()->SpawnActor<AOrbit>(OrbitClass);
		const auto GI = GetGameInstance<UMyGameInstance>();
		const auto VecR = GetActorLocation() - GI->VecF1;
		const auto VecV = FVector(0, 0, 1).Cross(VecR).GetSafeNormal() * sqrt(GI->Alpha / VecR.Length());
		Orbit->SetupActorInSpace(this, GI->VecF1, VecV);
		Orbit->UpdateOrbit(GI->Alpha, GI->WorldRadius);
    }
}
