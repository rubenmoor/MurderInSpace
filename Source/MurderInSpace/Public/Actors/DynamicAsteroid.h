#pragma once

#include "CoreMinimal.h"
#include "MyActor_StaticMesh.h"
#include "MyComponents/DynamicAsteroidMeshComponent.h"

#include "DynamicAsteroid.generated.h"

UCLASS()
class MURDERINSPACE_API ADynamicAsteroid final : public AMyActor_StaticMesh
{
    GENERATED_BODY()

public:
    ADynamicAsteroid();

protected:
    virtual void OnConstruction(const FTransform& Transform) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UDynamicAsteroidMeshComponent> DynamicAsteroidMesh;
};
