#include "Actors/DynamicAsteroid.h"

#include "MyComponents/DynamicAsteroidMeshComponent.h"

ADynamicAsteroid::ADynamicAsteroid()
{
    DynamicAsteroidMesh = CreateDefaultSubobject<UDynamicAsteroidMeshComponent>("DynamicAsteroidMesh");
    DynamicAsteroidMesh->SetupAttachment(Root);
}

void ADynamicAsteroid::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    StaticMesh->SetStaticMesh(DynamicAsteroidMesh->MakeStaticMesh());
}
