#include "SpaceItems/HandThruster.h"

#include "NiagaraComponent.h"

AHandThruster::AHandThruster(): AActor()
{
    PrimaryActorTick.bCanEverTick = false;

    StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
    SetRootComponent(StaticMeshComponent);

    NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>("NiagaraComponent");
    NiagaraComponent->SetupAttachment(StaticMeshComponent, "SocketFlame");
    NiagaraComponent->SetHiddenInGame(true);
}

void AHandThruster::EnableBurn(bool InBEnabled)
{
    NiagaraComponent->SetHiddenInGame(!InBEnabled);
}

void AHandThruster::SetEnableOverlap(bool InBEnabled)
{
    StaticMeshComponent->SetGenerateOverlapEvents(InBEnabled);
}

void AHandThruster::BeginPlay()
{
    Super::BeginPlay();
}

