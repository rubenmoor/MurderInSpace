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
class MURDERINSPACE_API ADynamicAsteroid final : public AMyActor_RealtimeMesh
{
    GENERATED_BODY()

public:
    ADynamicAsteroid();

    // parameters for OnGenerateMesh_Implementation, passed in by `AsteroidBelt`
    UPROPERTY(EditAnywhere, Category="Generation")
    float SizeParam = 500.;
    
    FRandomStream RandomStream;
    
protected:
    UPROPERTY(EditAnywhere, Category="Generation")
    TArray<FMaterialType> MaterialTypes;
    
    UPROPERTY(EditAnywhere, Category="Generation")
    double CollisionCapsuleRelativeSize = 0.9;

    // event handlers
    virtual void OnGenerateMesh_Implementation() override;

    // private methods
    UFUNCTION(BlueprintPure)
    UMaterialInstance* SelectMaterial();

    // simplex noise parameters

    // noise frequency = frequency factor / size parameter
    UPROPERTY(EditAnywhere, Category="Generation")
    double SxFrequencyFactor = 0.5;

    // noise amplitude = amplitude factor * Size parameter
    UPROPERTY(EditAnywhere, Category="Generation")
    double SxAmplitudeFactor = 0.5;
    
    UPROPERTY(EditAnywhere, Category="Generation")
    float SxLacunarity = 2.3;
    
    UPROPERTY(EditAnywhere, Category="Generation")
    float SxPersistance = 0.6;
    
    UPROPERTY(EditAnywhere, Category="Generation")
    int SxOctaves = 4;
};
