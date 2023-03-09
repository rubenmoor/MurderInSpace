#pragma once

#include "CoreMinimal.h"
#include "MyActor_StaticMesh.h"
#include "Components/DynamicMeshComponent.h"

#include "DynamicAsteroid.generated.h"

UCLASS()
class MURDERINSPACE_API ADynamicAsteroid final : public AMyActor_StaticMesh
{
    GENERATED_BODY()

public:
    ADynamicAsteroid();

    UFUNCTION()
    void GenerateMesh(float SizeParam);

protected:

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UDynamicMeshComponent> DynamicMeshComponent;

    // event handlers
    virtual void OnConstruction(const FTransform& Transform) override;
};
