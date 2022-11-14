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

void AMyActor_StaticMesh::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

	if(HasAnyFlags(RF_Transient))
	{
		return;
	}
		
	if(!OrbitClass)
	{
		UE_LOG(LogMyGame, Error, TEXT("%s: OnConstruction: OrbitClass null"), *GetFullName())
		return;
	}
	const FString NewName = FString(TEXT("Orbit_")).Append(GetFName().ToString());
	if(!IsValid(Orbit))
	{
		UE_LOG(LogMyGame, Warning, TEXT("%s: OnConstruction: NewName "), *NewName)
		Orbit = AOrbit::SpawnOrbit(this, NewName);
		Orbit->SetEnableVisibility(true);
	}
	else
	{
		// fix orbit after copying (alt + move in editor)
		if(Orbit->GetBody() != this)
		{
			UObject* Object = StaticFindObjectFastSafe(AOrbit::StaticClass(), GetWorld(), FName(NewName), true);
			// debugging: cast fails
			Orbit = Object ? Cast<AOrbit>(Object) : nullptr;
		}
		Orbit->OnConstruction(FTransform());
	}
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
