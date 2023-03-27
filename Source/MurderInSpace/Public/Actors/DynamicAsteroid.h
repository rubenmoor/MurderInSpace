﻿#pragma once

#include "CoreMinimal.h"
#include "MyActor_RealtimeMesh.h"
#include "Materials/MaterialInstanceConstant.h"

#include "DynamicAsteroid.generated.h"

USTRUCT(BlueprintType)
struct FFractureInfo
{
    GENERATED_BODY()

    FRealtimeMeshSimpleMeshData MeshData;
    FVector Location;
};

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

    void GenerateInitialMesh(float SizeParam, FRandomStream RandomStream); 

    UFUNCTION(CallInEditor, BlueprintCallable, Category="Generation")
    void Fracture();

    UFUNCTION(BlueprintCallable)
    void SetMeshData(FRealtimeMeshSimpleMeshData InMeshData) { MeshData = InMeshData; }

protected:
    UPROPERTY(EditAnywhere, Category="Generation")
    TArray<FMaterialType> MaterialTypes;
    
    UPROPERTY(EditAnywhere, Category="Generation")
    double CollisionCapsuleRelativeSize = 0.9;

    // event handlers
    virtual void OnGenerateMesh_Implementation() override;

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

    FRealtimeMeshSimpleMeshData MeshData;

    // private methods
    UFUNCTION(BlueprintPure)
    UMaterialInstance* SelectMaterial(float SizeParam, FRandomStream RandomStream);

    UFUNCTION(BlueprintCallable)
    TArray<FFractureInfo> GetFractures();  
    
};
