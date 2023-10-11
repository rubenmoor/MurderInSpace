#pragma once

#include "CoreMinimal.h"
#include "FastNoiseWrapper.h"
#include "MyActor_RealtimeMesh.h"
#include "RealtimeMeshSimple.h"
#include "GameplayAbilitySystem/GA_Embrace.h"
#include "Materials/MaterialInstanceConstant.h"
#include "MyComponents/GyrationComponent.h"

#include "DynamicAsteroid.generated.h"

class AAsteroidBelt;

USTRUCT(BlueprintType)
struct FFractureInfo
{
    GENERATED_BODY()

    FRealtimeMeshSimpleMeshData MeshData;
    FVector Location;
};

USTRUCT(BlueprintType)
struct FAsteroidType
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    TObjectPtr<UMaterialInstanceConstant> Material;

    UPROPERTY(EditAnywhere)
    double MinSize = 0.;
    
    UPROPERTY(EditAnywhere)
    double MaxSize = 1e11;

    UPROPERTY(EditAnywhere)
    double CoR = 1.;
};

USTRUCT()
struct FSelectedAsteroidType
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere)
    UMaterialInstanceConstant* Material;

    UPROPERTY(EditAnywhere)
    double CoR;
};

UENUM()
enum class EDynamicAsteroidOrigin : uint8
{
      SelfGenerated = 0
    , FromMeshData = 1
};

UCLASS()
class MURDERINSPACE_API ADynamicAsteroid final
    : public AMyActor_RealtimeMesh
    , public IHasGyration
    , public ICanBeEmbraced
{
    GENERATED_BODY()

public:
    ADynamicAsteroid();

    UFUNCTION(BlueprintCallable)
    void Initialize
        ( float InSizeParam
        , float InMeshResolution
        , bool InBRecomputeNormals
        , int32 InSeed
        , AAsteroidBelt* InAsteroidBelt
        )
    {
        RP_SizeParam = InSizeParam;
        RP_MeshResolution = InMeshResolution;
        RP_bRecomputeNormals = InBRecomputeNormals;
        RP_Seed = InSeed;
        RandomStream.Initialize(RP_Seed);
        AsteroidBelt = InAsteroidBelt;
    }

    UFUNCTION(CallInEditor, BlueprintCallable, Category="Generation")
    void Fracture();

    UFUNCTION(BlueprintCallable)
    void SetMeshData(FRealtimeMeshSimpleMeshData InMeshData)
    {
        Origin = EDynamicAsteroidOrigin::FromMeshData;
        MeshData = InMeshData;
    }

    EDynamicAsteroidOrigin Origin = EDynamicAsteroidOrigin::SelfGenerated;

    UPROPERTY(VisibleAnywhere, Category="Generation")
    AAsteroidBelt* AsteroidBelt = nullptr;

    virtual FVector GetInitialOmega() override;

    // ICanBeEmbraced
    virtual float GetRadius() override;

protected:
    UPROPERTY(EditDefaultsOnly, Category="Generation")
    TArray<FAsteroidType> AsteroidTypes;
    
    UPROPERTY(EditAnywhere, Category="Generation")
    double CollisionCapsuleRelativeSize = 0.9;

    // event handlers
    virtual void OnGenerateMesh_Implementation() override;
    virtual void Destroyed() override;
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;

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
    UPROPERTY(EditAnywhere, Replicated, Category="Generation")
    int32 RP_Seed = 0;
    
    UPROPERTY(EditAnywhere, Replicated, Category="Generation")
    float RP_MeshResolution = 1.;

    UPROPERTY(EditAnywhere, Replicated, Category="Generation")
    bool RP_bRecomputeNormals = false;

    UPROPERTY(EditAnywhere, Replicated, Category="Generation")
    float RP_SizeParam = 100;

    UPROPERTY(EditDefaultsOnly, Category="Generation")
    TObjectPtr<UCurveFloat> CurveOmegaDistribution;

    // angular velocity maximum, used with `CurveOmegaDistribution` in `GetInitialOmega` to initialize gyration
    UPROPERTY(EditDefaultsOnly, Category="Generation")
    double OmegaMax = 0.1;

    FRealtimeMeshSimpleMeshData MeshData;

    // private methods

    // fill MeshData with asteroid mesh
    void GenerateAsteroid();

    // returning structs is not supported by blueprint
    UFUNCTION()
    FSelectedAsteroidType SelectAsteroidType();

    UFUNCTION(BlueprintCallable)
    TArray<FFractureInfo> GetFractures();

protected:
    FRandomStream RandomStream;

    UPROPERTY()
    TObjectPtr<UFastNoiseWrapper> FastNoiseWrapper;
};
