#include "ActorInSpace.h"

#include "GyrationComponent.h"
#include "OrbitComponent.h"

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

	Gyration = CreateDefaultSubobject<UGyrationComponent>(TEXT("Gyration"));
}

void AActorInSpace::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Orbit->InitializeCircle(MovableRoot->GetComponentLocation(), AMyGameState::DefaultSpaceParams);
}

void AActorInSpace::HandleBeginMouseOver(UPrimitiveComponent*)
{
	Orbit->SetVisibility(true, true);
	Orbit->bIsVisible = true;
}

void AActorInSpace::HandleEndMouseOver(UPrimitiveComponent*)
{
	if(!Orbit->bIsSelected)
	{
		Orbit->SetVisibility(false, true);
		Orbit->bIsVisible = false;
	}
}

void AActorInSpace::HandleClick(UPrimitiveComponent*, FKey Button)
{
	if(Button == EKeys::LeftMouseButton)
	{
		Orbit->bIsSelected = !Orbit->bIsSelected;
	}
}
