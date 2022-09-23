#include "ActorInSpace.h"

#include "MyGameInstance.h"

AActorInSpace::AActorInSpace()
{
	PrimaryActorTick.bCanEverTick = true;

	// components

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Root->SetMobility(EComponentMobility::Stationary);
	SetRootComponent(Root);

	MovableRoot = CreateDefaultSubobject<USceneComponent>(TEXT("MovableRoot"));
	MovableRoot->SetupAttachment(Root);
	
	Orbit = CreateDefaultSubobject<UOrbitComponent>(TEXT("Orbit"));
	Orbit->SetupAttachment(Root);
	Orbit->SetMovableRoot(MovableRoot);
}

void AActorInSpace::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Orbit->InitializeCircle
		( UMyGameInstance::EditorDefaultAlpha
		, UMyGameInstance::EditorDefaultWorldRadiusUU
		, UMyGameInstance::EditorDefaultVecF1
		, MovableRoot->GetComponentLocation()
		);
}
