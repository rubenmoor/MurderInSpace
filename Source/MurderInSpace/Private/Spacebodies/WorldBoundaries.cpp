#include "Spacebodies/WorldBoundaries.h"

#include "Modes/MyGameState.h"

// Sets default values
AWorldBoundaries::AWorldBoundaries(): AActor()
{
	PrimaryActorTick.bCanEverTick = false;
	AActor::SetActorHiddenInGame(true);

	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	Sphere->SetMobility(EComponentMobility::Stationary);
	SetRootComponent(Sphere);
}

void AWorldBoundaries::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Sphere->SetSphereRadius(FPhysics::MAX_WORLDRADIUS_UU);
}

// Called when the game starts or when spawned
void AWorldBoundaries::BeginPlay()
{
	Super::BeginPlay();
	auto* GS = AMyGameState::Get(this);
	Sphere->SetSphereRadius(GS->RP_Physics.WorldRadius);
}

// Called every frame
void AWorldBoundaries::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

