#pragma once

#include "CoreMinimal.h"
#include "MyActor_RealtimeMesh.h"
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
class MURDERINSPACE_API ADynamicAsteroid final : public AMyActor_RealtimeMesh
{
    GENERATED_BODY()

public:
    ADynamicAsteroid();

    UFUNCTION()
    // TODO: deprecated
    void GenerateMesh(FRandomStream RandomStream, double SizeParam);

protected:
    UPROPERTY(EditAnywhere, Category="Generation")
    TArray<FMaterialType> MaterialTypes;
    
    UPROPERTY(EditAnywhere, Category="Generation")
    double CollisionCapsuleRelativeSize = 0.9;

    // event handlers
    virtual void OnGenerateMesh_Implementation() override;

    // private methods
    UFUNCTION(BlueprintPure)
    UMaterialInstance* SelectMaterial(FRandomStream RandomStream, double SizeParam);
};
