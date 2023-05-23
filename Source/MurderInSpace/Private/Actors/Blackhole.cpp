#include "Actors/Blackhole.h"

ABlackhole::ABlackhole()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>("SceneRoot");
	SetRootComponent(Root);

	EventHorizon = CreateDefaultSubobject<UStaticMeshComponent>("EventHorizon");
	GravitationalLens = CreateDefaultSubobject<UStaticMeshComponent>("GravitationalLens");

	EventHorizon->SetupAttachment(Root);
	GravitationalLens->SetupAttachment(Root);
}
