// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/MyActor_StaticMesh.h"

#include "Actors/GyrationComponent.h"

AMyActor_StaticMesh::AMyActor_StaticMesh()
{
    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);
    
    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
    StaticMesh->SetupAttachment(Root);
    
    Gyration = CreateDefaultSubobject<UGyrationComponent>(TEXT("Gyration"));
}

void AMyActor_StaticMesh::InitializeOrbit()
{
	SpawnOrbit(this);
	Orbit->SetEnableVisibility(true);
}

void AMyActor_StaticMesh::Destroyed()
{
	Super::Destroyed();
	if(!IsValid(Orbit))
	{
		UE_LOG(LogMyGame, Warning, TEXT("%s: BeginDestroy: orbit null"), *GetFullName())
	}
	else
	{
		Orbit->Destroy();
	}
}

void AMyActor_StaticMesh::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
	OrbitOnConstruction(this, true);
}

void AMyActor_StaticMesh::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
	
    const FPhysics Physics = UStateLib::GetPhysicsEditorDefault();
    const FInstanceUI InstanceUI = UStateLib::GetInstanceUIEditorDefault();
    
    const FName Name = PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName();

    static const FName FNameOrbitColor = GET_MEMBER_NAME_CHECKED(AMyActor_StaticMesh, OrbitColor);

	if(Name == FNameOrbitColor)
	{
		if(IsValid(Orbit))
		{
			Orbit->Update(Physics, InstanceUI);
		}
	}
}
