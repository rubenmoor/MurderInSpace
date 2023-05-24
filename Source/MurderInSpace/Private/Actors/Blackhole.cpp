#include "Actors/Blackhole.h"

#include "NiagaraComponent.h"

ABlackhole::ABlackhole()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>("SceneRoot");
	SetRootComponent(Root);

	EventHorizon = CreateDefaultSubobject<UStaticMeshComponent>("EventHorizon");
	EventHorizon->SetupAttachment(Root);

	GravitationalLens = CreateDefaultSubobject<UStaticMeshComponent>("GravitationalLens");
	GravitationalLens->SetupAttachment(Root);

	NS_Vortex = CreateDefaultSubobject<UNiagaraComponent>("Vortex");
	NS_Vortex->SetupAttachment(Root);
	NS_Vortex->SetVariableFloat("Albedo", 0.5);
	NS_Vortex->SetVariableFloat("Extinction", 0.1);
	NS_Vortex->SetVariableLinearColor("Color", FLinearColor(FVector4d(1., 0.772553, 0., 1.)));
	NS_Vortex->SetVariableVec2("MinSize", FVector2D(50., 10.));
	NS_Vortex->SetVariableVec2("MaxSize", FVector2D(100., 250.));

	NS_Jet = CreateDefaultSubobject<UNiagaraComponent>("Jet");
	NS_Jet->SetupAttachment(Root);
	
	NS_Jetstream = CreateDefaultSubobject<UNiagaraComponent>("Jetstream");
	NS_Jetstream->SetupAttachment(Root);
}
