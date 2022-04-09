#include "AObjectInSpace.h"

#include "MyGameInstance.h"
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
	
	//Orbit = new UKeplerOrbitComponent(FVector::Zero(), MeshRoot);
	Orbit = CreateDefaultSubobject<UKeplerOrbitComponent>(TEXT("KeplerOrbit"));
	Orbit->Initialize(FVector::Zero(), MeshRoot);
	Orbit->SetupAttachment(Root);
}

void AAObjectInSpace::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAObjectInSpace::UpdateMU(float MU) const
{
	Orbit->UpdateOrbit(MU);
}

void AAObjectInSpace::BeginPlay()
{
	Super::BeginPlay();
}

void AAObjectInSpace::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Orbit->UpdateOrbit(UKeplerOrbitComponent::DefaultMU);
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
