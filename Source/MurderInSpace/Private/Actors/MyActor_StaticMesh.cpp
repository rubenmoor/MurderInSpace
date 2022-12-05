// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/MyActor_StaticMesh.h"

#include "Actors/GyrationComponent.h"

AMyActor_StaticMesh::AMyActor_StaticMesh()
{
	bNetLoadOnClient = false;
	bReplicates = true;
    bAlwaysRelevant = true;
	AActor::SetReplicateMovement(false);
	
    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);
    
    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
    StaticMesh->SetupAttachment(Root);
    
    Gyration = CreateDefaultSubobject<UGyrationComponent>(TEXT("Gyration"));
}

void AMyActor_StaticMesh::Destroyed()
{
	Super::Destroyed();
	while(Children.Num() > 0)
	{
		if(IsValid(Children.Last()))
		{
			Children.Last()->Destroy();
		}
	}
}

void AMyActor_StaticMesh::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
	OrbitSetup(this);
}

#if WITH_EDITOR
void AMyActor_StaticMesh::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

    const FName Name = PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName();

    static const FName FNameOrbitColor = GET_MEMBER_NAME_CHECKED(AMyActor_StaticMesh, OrbitColor);

	if(Name == FNameOrbitColor)
	{
		if(!Children.IsEmpty())
		{
			Cast<AOrbit>(Children[0])->Update(PhysicsEditorDefault, InstanceUIEditorDefault);
		}
	}
}
#endif
