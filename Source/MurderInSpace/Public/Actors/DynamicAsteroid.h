#pragma once

#include "CoreMinimal.h"
#include "FastNoiseWrapper.h"
#include "MyActor_RealtimeMesh.h"
#include "RealtimeMeshSimple.h"
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

UENUM()
enum class EDynamicAsteroidOrigin : uint8
{
      SelfGenerated = 0
    , FromMeshData = 1
};

UCLASS()
class MURDERINSPACE_API ADynamicAsteroid final : public AMyActor_RealtimeMesh, public IHasRandom
{
    GENERATED_BODY()

public:
    ADynamicAsteroid();

    UFUNCTION(BlueprintCallable)
    void Initialize(float InSizeParam, float InMeshResolution, bool InBRecomputeNormals, int32 InSeed)
    {
        SizeParam = InSizeParam;
        MeshResolution = InMeshResolution;
        bRecomputeNormals = InBRecomputeNormals;
        RandomStream.Initialize(InSeed);
    }

    UFUNCTION(CallInEditor, BlueprintCallable, Category="Generation")
    void Fracture();

    UFUNCTION(BlueprintCallable)
    void SetMeshData(FRealtimeMeshSimpleMeshData InMeshData)
    {
        Origin = EDynamicAsteroidOrigin::FromMeshData;
        MeshData = InMeshData;
    }

    virtual int32 GetSeed() override { return RandomStream.GetUnsignedInt(); }

    EDynamicAsteroidOrigin Origin = EDynamicAsteroidOrigin::SelfGenerated;

protected:
    UPROPERTY(EditAnywhere, Category="Generation")
    TArray<FMaterialType> MaterialTypes;
    
    UPROPERTY(EditAnywhere, Category="Generation")
    double CollisionCapsuleRelativeSize = 0.9;

    // event handlers
    virtual void OnGenerateMesh_Implementation() override;

    // fast noise, noise parameters

    UPROPERTY(EditAnywhere, Category="Generation|Distortion")
    EFastNoise_NoiseType NoiseType = EFastNoise_NoiseType::Simplex; 

    UPROPERTY(EditAnywhere, Category="Generation|Distortion")
    int32 NoiseSeed = 0;
    
    // noise frequency = frequency factor / size parameter
    UPROPERTY(EditAnywhere, Category="Generation|Distortion")
    double FrequencyFactor = 0.5;

    UPROPERTY(EditAnywhere, Category="Generation|Distortion")
    EFastNoise_Interp Interp = EFastNoise_Interp::Quintic;
    
    UPROPERTY(EditAnywhere, Category="Generation|Distortion")
    EFastNoise_FractalType FractalType = EFastNoise_FractalType::FBM;
    
    // noise amplitude = amplitude factor * Size parameter
    UPROPERTY(EditAnywhere, Category="Generation|Distortion")
    double AmplitudeFactor = 0.5;
    
    UPROPERTY(EditAnywhere, Category="Generation|Distortion")
    float Lacunarity = 2.3;
    
    UPROPERTY(EditAnywhere, Category="Generation|Distortion")
    float Gain = 0.6;
    
    UPROPERTY(EditAnywhere, Category="Generation|Distortion")
    int Octaves = 4;

    // mesh generation
    
    UPROPERTY(EditAnywhere, Category="Generation")
    float MeshResolution = 1.;

    UPROPERTY(EditAnywhere, Category="Generation")
    bool bRecomputeNormals = false;

    UPROPERTY(EditAnywhere, Category="Generation")
    float SizeParam = 500;

    FRealtimeMeshSimpleMeshData MeshData;

    // private methods

    // fill MeshData with asteroid mesh
    void GenerateAsteroid();

    UFUNCTION(BlueprintPure)
    UMaterialInstance* SelectMaterial();

    UFUNCTION(BlueprintCallable)
    TArray<FFractureInfo> GetFractures();  

    FRandomStream RandomStream;

    UPROPERTY()
    TObjectPtr<UFastNoiseWrapper> FastNoiseWrapper;
};
