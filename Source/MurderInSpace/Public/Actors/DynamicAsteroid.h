#pragma once

#include "CoreMinimal.h"
#include "MyActor_StaticMesh.h"
#include "RealtimeMeshComponent.h"
#include "Components/DynamicMeshComponent.h"
#include "Materials/MaterialInstanceConstant.h"

#include "DynamicAsteroid.generated.h"

USTRUCT(BlueprintType)
struct FMaterialType
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    TObjectPtr<UMaterialInstanceConstant> Material;

    UPROPERTY(EditAnywhere)
    double MinSize = 0.;
    
    UPROPERTY(EditAnywhere)
    double MaxSize = 1e11;
};

UCLASS()
// TODO: inherit from AActor
class MURDERINSPACE_API ADynamicAsteroid final : public AMyActor_StaticMesh
{
    GENERATED_BODY()

public:
    ADynamicAsteroid();

    UFUNCTION()
    void GenerateMesh(FRandomStream RandomStream, double SizeParam);

protected:
    UPROPERTY(EditAnywhere, Category="Generation")
    TArray<FMaterialType> MaterialTypes;
    
    UPROPERTY(EditAnywhere, Category="Generation")
    double CollisionCapsuleRelativeSize = 0.9;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<URealtimeMeshComponent> RealtimeMeshComponent;

    // event handlers
    virtual void OnConstruction(const FTransform& Transform) override;

    // private methods
    UFUNCTION(BlueprintPure)
    UMaterialInstance* SelectMaterial(FRandomStream RandomStream, double SizeParam);
};
