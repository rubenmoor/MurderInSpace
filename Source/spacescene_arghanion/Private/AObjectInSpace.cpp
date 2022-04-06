#include "AObjectInSpace.h"

#include "FunctionLib.h"
#include "GameModeAsteroids.h"
#include "VectorTypes.h"
#include "Kismet/GameplayStatics.h"

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

AAObjectInSpace::AAObjectInSpace()
{
	PrimaryActorTick.bCanEverTick = true;

	// components

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(Root);

	MeshRoot = CreateDefaultSubobject<USceneComponent>(TEXT("MeshRoot"));
	MeshRoot->SetupAttachment(Root);
	
	MainMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainMesh"));
	MainMesh->SetupAttachment(MeshRoot);

	//Orbit = new UKeplerOrbitComponent(FVector::Zero(), MeshRoot);
	Orbit = CreateDefaultSubobject<UKeplerOrbitComponent>(TEXT("KeplerOrbit"));
	Orbit->Initialize(FVector::Zero(), MeshRoot);
	Orbit->SetupAttachment(Root);
}

void AAObjectInSpace::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAObjectInSpace::BeginPlay()
{
	Super::BeginPlay();
}

void AAObjectInSpace::OnConstruction(const FTransform& Transform)
{
	const auto MU = Cast<AGameModeAsteroids>(UGameplayStatics::GetGameMode(GetWorld()))->MU;
	const auto VecR = Transform.GetLocation();
	const auto VecV = 0.8 * sqrt(MU / VecR.Length()) * (FVector(.2, 0, 0) + FVector(0.1, 0.2, 0.9).Cross(VecR.GetSafeNormal()));
	Orbit->UpdateOrbit(VecR, VecV);
	Super::OnConstruction(Transform);
}
