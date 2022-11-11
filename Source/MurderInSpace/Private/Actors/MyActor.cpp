#include "Actors/MyActor.h"

#include "Actors/GyrationComponent.h"
#include "Actors/OrbitComponent.h"

AMyActor::AMyActor()
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

    SplineMeshParent = CreateDefaultSubobject<USceneComponent>(TEXT("SplineMesh"));
    SplineMeshParent->SetupAttachment(Orbit);
    SplineMeshParent->SetMobility(EComponentMobility::Stationary);
    
    bNetLoadOnClient = false;
    bReplicates = true;
    AActor::SetReplicateMovement(false);
}

void AMyActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
	IHasOrbit::Construction(Transform);
}

