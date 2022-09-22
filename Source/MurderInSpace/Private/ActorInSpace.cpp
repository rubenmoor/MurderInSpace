#include "ActorInSpace.h"

#include "MyGameInstance.h"

AActorInSpace::AActorInSpace()
{
	PrimaryActorTick.bCanEverTick = true;

	// components

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Root->SetMobility(EComponentMobility::Stationary);
	SetRootComponent(Root);

	Orbit = CreateDefaultSubobject<UOrbitComponent>(TEXT("Orbit"));
	Orbit->SetupAttachment(Root);
	
	MovableRoot = CreateDefaultSubobject<USceneComponent>(TEXT("MovableRoot"));
	MovableRoot->SetupAttachment(Root);
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

void AActorInSpace::BeginPlay()
{
	Super::BeginPlay();
	const UMyGameInstance* GI = GetWorld()->GetGameInstance<UMyGameInstance>();
	Orbit->InitializeCircle(GI->Alpha, GI->WorldRadius, GI->VecF1, MovableRoot->GetComponentLocation());
}

void AActorInSpace::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const FVector NewLocation = Orbit->GetNextLocation(DeltaSeconds);
	MovableRoot->SetWorldLocation(NewLocation, true, nullptr, ETeleportType::TeleportPhysics);
}
