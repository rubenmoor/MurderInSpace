#include "AObjectInSpace.h"

#include "MyGameInstance.h"
#include "VectorTypes.h"
#include "Kismet/GameplayStatics.h"

AAObjectInSpace::AAObjectInSpace()
{
	PrimaryActorTick.bCanEverTick = true;

	// components

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(Root);

	MeshRoot = CreateDefaultSubobject<USceneComponent>(TEXT("MeshRoot"));
	MeshRoot->SetupAttachment(Root);

	TrajectoryMesh = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("Trajectory Mesh"));
	TrajectoryMesh->SetupAttachment(Root);
	
	//Orbit = new UKeplerOrbitComponent(FVector::Zero(), MeshRoot);
	Orbit = CreateDefaultSubobject<UKeplerOrbitComponent>(TEXT("KeplerOrbit"));
	Orbit->Initialize(FVector::Zero(), MeshRoot, TrajectoryMesh);
	Orbit->SetupAttachment(Root);
}

void AAObjectInSpace::UpdateMU(float MU, float RMAX) const
{
	Orbit->UpdateOrbit(MU, RMAX);
}

void AAObjectInSpace::BeginPlay()
{
	const auto GameInstance = GetGameInstance<UMyGameInstance>();
	Orbit->UpdateOrbit(GameInstance->MU, GameInstance->GameAreaRadius);
	Super::BeginPlay();
}

void AAObjectInSpace::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Orbit->UpdateOrbit(UKeplerOrbitComponent::DefaultMU, UMyGameInstance::DefaultGameAreaRadius);
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
