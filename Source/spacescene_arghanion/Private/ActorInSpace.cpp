#include "ActorInSpace.h"

#include "MyGameInstance.h"
#include "Kismet/GameplayStatics.h"

AActorInSpace::AActorInSpace()
{
	PrimaryActorTick.bCanEverTick = true;

	// components

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(Root);

	// if(Orbit)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("AActorInSpace::AActorInSpace: Orbit not null"))
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
	// 			UE_LOG(LogTemp, Warning, TEXT("AActorInSpace::AActorInSpace: OrbitClass null"))
	// 		}
	// 	}
	// 	else
	// 	{
	// 		UE_LOG(LogTemp, Warning, TEXT("AActorInSpace::AActorInSpace: World null"))
	// 	}
	// }
}

void AActorInSpace::UpdateMU(float MU, float RMAX) const
{
	Orbit->UpdateOrbit(MU, RMAX);
}

void AActorInSpace::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	static const FName FNameOrbit = GET_MEMBER_NAME_CHECKED(AActorInSpace, Orbit);
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

void AActorInSpace::SpawnOrbit()
{
 	if(Orbit)
 	{
 		UE_LOG(LogTemp, Warning, TEXT("AActorInSpace::SpawnOrbit: Orbit already set; skipping"))
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
