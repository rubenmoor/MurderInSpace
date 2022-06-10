#include "ActorInSpace.h"

#include "OrbitDataComponent.h"
#include "Kismet/GameplayStatics.h"

AActorInSpace::AActorInSpace()
{
	PrimaryActorTick.bCanEverTick = false;

	// components

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(Root);

	OrbitData = CreateDefaultSubobject<UOrbitDataComponent>(TEXT("Orbit Data"));
}