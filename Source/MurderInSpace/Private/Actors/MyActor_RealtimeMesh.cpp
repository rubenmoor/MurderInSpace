#include "Actors/MyActor_RealtimeMesh.h"

#include "MyComponents/GyrationComponent.h"
#include "MyComponents/MyCollisionComponent.h"
#include "Net/UnrealNetwork.h"

AMyActor_RealtimeMesh::AMyActor_RealtimeMesh()
{
    bNetLoadOnClient = false;
    bReplicates = true;
    bAlwaysRelevant = true;
    AActor::SetReplicateMovement(false);

    RealtimeMeshComponent->CanCharacterStepUpOn = ECB_No;
    
    Gyration = CreateDefaultSubobject<UGyrationComponent>("Gyration");
    Collision = CreateDefaultSubobject<UMyCollisionComponent>("Collision");
}

void AMyActor_RealtimeMesh::Destroyed()
{
    Super::Destroyed();
    if(IsValid(RP_Orbit))
    {
        RP_Orbit->Destroy();
    }
}

void AMyActor_RealtimeMesh::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    ensureMsgf(GetLocalRole() == ROLE_Authority, TEXT("%s: OnConstruction while Role < Authority"), *GetFullName());

    // TODO: if this actor is restored from a save game: no orbit setup
    if  (
        GetWorld()->WorldType != EWorldType::EditorPreview
        
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

#if WITH_EDITOR
void AMyActor_RealtimeMesh::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
    Super::PostEditChangeChainProperty(PropertyChangedEvent);
    
    const FName Name = PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName();

    static const FName FNameOrbitColor    = GET_MEMBER_NAME_CHECKED(AMyActor_RealtimeMesh, OrbitColor        );
    static const FName FNameInitialParams = GET_MEMBER_NAME_CHECKED(AMyActor_RealtimeMesh, InitialOrbitParams);

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

void AMyActor_RealtimeMesh::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(AMyActor_RealtimeMesh, RP_Orbit, COND_InitialOnly)
}

