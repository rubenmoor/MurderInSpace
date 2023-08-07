#include "SpaceItems/HandThruster.h"

#include "NiagaraComponent.h"

AHandThruster::AHandThruster(): AActor()
{
    PrimaryActorTick.bCanEverTick = false;

    StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
    SetRootComponent(StaticMeshComponent);

    NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>("NiagaraComponent");
    NiagaraComponent->SetupAttachment(StaticMeshComponent, "SocketFlame");
    NiagaraComponent->SetVisibility(false);
}

void AHandThruster::EnableBurn(bool InBEnabled)
{
    NiagaraComponent->SetVisibility(InBEnabled);
}

void AHandThruster::BeginPlay()
{
    Super::BeginPlay();
}

