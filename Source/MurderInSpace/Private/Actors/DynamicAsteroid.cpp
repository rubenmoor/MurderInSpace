#include "Actors/DynamicAsteroid.h"

#include "MyComponents/DynamicAsteroidMeshComponent.h"

ADynamicAsteroid::ADynamicAsteroid()
{
    DynamicAsteroidMesh = CreateDefaultSubobject<UDynamicAsteroidMeshComponent>("DynamicAsteroidMesh");
    DynamicAsteroidMesh->SetupAttachment(Root);
}

void ADynamicAsteroid::OnConstruction(const FTransform& Transform)
{
    StaticMesh->SetStaticMesh(DynamicAsteroidMesh->MakeStaticMesh());
    DynamicAsteroidMesh->DestroyComponent();
    
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

