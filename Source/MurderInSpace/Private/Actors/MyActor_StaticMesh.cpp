// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/MyActor_StaticMesh.h"

#include "Actors/GyrationComponent.h"
#include "Lib/FunctionLib.h"

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

	if(HasAnyFlags(RF_Transient | RF_ClassDefaultObject))
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
		Orbit = AOrbit::SpawnOrbit(this, NewName);
		if(Orbit)
		{
			Orbit->SetEnableVisibility(true);
		}
	}
	else
	{
		// fix orbit after copying (alt + move in editor)
		if(Orbit->GetBody() != this)
		{
			// this should work, but it doesn't:
			//Orbit = FindObjectFast<AOrbit>(this, FName(*NewName));
			// so this is my workaround
			UWorld* World = GetWorld();
			auto Filter = [this, World] (const AOrbit* Orbit)
			{
				return Orbit->GetWorld() == World
					&& Orbit->GetBody() == this;
			};
			for(MyObjectIterator<AOrbit> IOrbit(Filter); IOrbit; ++IOrbit)
			{
				Orbit = *IOrbit;
			}
			if(!IsValid(Orbit))
			{
				UE_LOG(LogMyGame, Error, TEXT("%s: couldn't find AObject by fname %s"), *GetFullName(), *NewName)
			}
		}
		if(IsValid(Orbit))
		{
			Orbit->OnConstruction(FTransform());
#if WITH_EDITORONLY_DATA
			SetActorLabel(GetFName().ToString());
#endif
		}
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
