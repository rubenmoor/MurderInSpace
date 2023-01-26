#include "Actors/MyActor_StaticMesh.h"

#include "MyComponents/GyrationComponent.h"
#include "MyComponents/MyCollisionComponent.h"
#include "Net/UnrealNetwork.h"

AMyActor_StaticMesh::AMyActor_StaticMesh()
{
	bNetLoadOnClient = false;
	bReplicates = true;
    bAlwaysRelevant = true;
	AActor::SetReplicateMovement(false);
	
    Root = CreateDefaultSubobject<USceneComponent>("Root");
    SetRootComponent(Root);
    
    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
    StaticMesh->SetupAttachment(Root);
    
    Gyration = CreateDefaultSubobject<UGyrationComponent>("Gyration");
    Collision = CreateDefaultSubobject<UMyCollisionComponent>("Collision");
}

void AMyActor_StaticMesh::Destroyed()
{
	Super::Destroyed();
	if(IsValid(RP_Orbit))
	{
		RP_Orbit->Destroy();
	}
}

void AMyActor_StaticMesh::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
	
    if  (
		// Only the server spawns orbits
    	   GetLocalRole()        == ROLE_Authority
    	   
		// avoid orbit spawning when editing and compiling blueprint
		&& GetWorld()->WorldType != EWorldType::EditorPreview
		
		// avoid orbit spawning when dragging an actor with orbit into the viewport at first
		// The preview actor that is created doesn't have a valid location
		// Once the actor is placed inside the viewport, it's no longer transient and the orbit is reconstructed properly
		// according to the actor location
		&& !HasAnyFlags(RF_Transient)
		)
    {
		OrbitSetup(this);
    }
}

void AMyActor_StaticMesh::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(AMyActor_StaticMesh, RP_Orbit      , COND_InitialOnly)
}

#if WITH_EDITOR
void AMyActor_StaticMesh::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

    const FName Name = PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName();

    static const FName FNameOrbitColor = GET_MEMBER_NAME_CHECKED(AMyActor_StaticMesh, OrbitColor);

	if(Name == FNameOrbitColor)
	{
		if(IsValid(RP_Orbit))
		{
			RP_Orbit->Update(PhysicsEditorDefault, InstanceUIEditorDefault);
		}
	}
}
#endif
