#include "Actors/DynamicAsteroid.h"

#include "MyComponents/DynamicAsteroidMeshComponent.h"

ADynamicAsteroid::ADynamicAsteroid()
{
    DynamicAsteroidMesh = CreateDefaultSubobject<UDynamicAsteroidMeshComponent>("DynamicAsteroidMesh");
    DynamicAsteroidMesh->SetupAttachment(Root);
}

void ADynamicAsteroid::OnConstruction(const FTransform& Transform)
{
    const float SizeParam = Transform.GetScale3D().X;
    SetActorScale3D(FVector::One());
    StaticMesh->SetStaticMesh(DynamicAsteroidMesh->MakeStaticMesh(SizeParam));
    // probably rather hide the dynamic mesh and use later for collision/destruction
    if(GetWorld()->WorldType != EWorldType::EditorPreview)
    {
        DynamicAsteroidMesh->DestroyComponent();
    }
    
    Super::OnConstruction(Transform);
}

#if WITH_EDITOR
void ADynamicAsteroid::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
    Super::PostEditChangeChainProperty(PropertyChangedEvent);
    
    const FName Name = PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName();

    static const FName FNameInitialOrbitParams = GET_MEMBER_NAME_CHECKED(ADynamicAsteroid, InitialOrbitParams);

    if(Name == FNameInitialOrbitParams)
    {
        bInitialOrbitParamsSet = true;
    }
}
#endif

