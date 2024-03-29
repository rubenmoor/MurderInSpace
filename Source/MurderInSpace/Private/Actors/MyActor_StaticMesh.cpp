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
    
    StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
    StaticMeshComponent->SetupAttachment(Root);

    Gyration = CreateDefaultSubobject<UGyrationComponent>("Gyration");
    CollisionComponent = CreateDefaultSubobject<UMyCollisionComponent>("Collision");
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

    // TODO: if this actor is restored from a save game: no orbit setup
    if  (
        GetWorld()->WorldType != EWorldType::EditorPreview
        
        // avoid orbit spawning when dragging an actor with orbit into the viewport at first
        // The preview actor that is created doesn't have a valid location
        // Once the actor is placed inside the viewport, it's no longer transient and the orbit is reconstructed properly
        // according to the actor location
        && !HasAnyFlags(RF_Transient)
        && GetLocalRole() == ROLE_Authority
        )
    {
        OrbitSetup(this);
    }
    CollisionComponent->UpdateMass(GetBounds().SphereRadius);
}

#if WITH_EDITOR
void AMyActor_StaticMesh::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
    Super::PostEditChangeChainProperty(PropertyChangedEvent);

    const FName Name = PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName();

    static const FName FNameOrbitColor    = GET_MEMBER_NAME_CHECKED(AMyActor_StaticMesh, OrbitColor        );
    static const FName FNameInitialParams = GET_MEMBER_NAME_CHECKED(AMyActor_StaticMesh, InitialOrbitParams);

    if(Name == FNameOrbitColor)
    {
        if(IsValid(RP_Orbit))
        {
            RP_Orbit->Update(PhysicsEditorDefault, InstanceUIEditorDefault);
        }
    }
    else if(Name == FNameInitialParams)
    {
        if(IsValid(RP_Orbit))
        {
            RP_Orbit->UpdateByInitialParams(PhysicsEditorDefault, InstanceUIEditorDefault);
        }
    }
}
#endif

void AMyActor_StaticMesh::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(AMyActor_StaticMesh, RP_Orbit, COND_InitialOnly)
}

