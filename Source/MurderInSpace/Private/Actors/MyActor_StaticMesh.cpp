// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/MyActor_StaticMesh.h"

#include "Actors/GyrationComponent.h"
#include "Modes/MyGameInstance.h"
#include "Tasks/GameplayTask_SpawnActor.h"

AMyActor_StaticMesh::AMyActor_StaticMesh()
{
    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);
    
    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
    StaticMesh->SetupAttachment(Root);
    
    Gyration = CreateDefaultSubobject<UGyrationComponent>(TEXT("Gyration"));
}

void AMyActor_StaticMesh::CreateOrbit()
{
#if WITH_EDITOR
    FPhysics Physics = UStateLib::GetPhysicsEditorDefault();
#else
    FPhysics Physics = UStateLib::GetPhysicsUnsafe(this);
#endif
    
    if(!Orbit)
    {
        Orbit = GetWorld()->SpawnActor<AOrbit>();
    }
    Orbit->SetCircleOrbit(GetActorLocation(), Physics);
}

void AMyActor_StaticMesh::BeginPlay()
{
    Super::BeginPlay();

    UMyGameInstance* GI = GetGameInstance<UMyGameInstance>();
    
    OnBeginCursorOver.AddDynamic(GI, &UMyGameInstance::HandleBeginMouseOver);
    OnEndCursorOver.AddDynamic(GI, &UMyGameInstance::HandleEndMouseOver);
    OnClicked.AddDynamic(GI, &UMyGameInstance::HandleClick);
}

void AMyActor_StaticMesh::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    if(Orbit)
    {
        FPhysics Physics = UStateLib::GetPhysicsEditorDefault();
        FInstanceUI InstanceUI = UStateLib::GetInstanceUIEditorDefault();
        Orbit->SetCircleOrbit(Transform.GetLocation(), Physics);
        Orbit->Update(Physics, InstanceUI);
    }
}
