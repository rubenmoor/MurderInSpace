#include "Spacebodies/DynamicAsteroid.h"

#include "KismetProceduralMeshLibrary.h"
#include "RealtimeMesh.h"
#include "RealtimeMeshLibrary.h"
#include "RealtimeMeshSimple.h"
#include "FastNoiseWrapper.h"
#include "ProceduralMeshComponent.h"
#include "Spacebodies/AsteroidBelt.h"
#include "Logging/StructuredLog.h"
#include "Modes/MyGameInstance.h"
#include "Modes/MyState.h"
#include "Net/UnrealNetwork.h"

ADynamicAsteroid::ADynamicAsteroid()
{
    RandomStream.Initialize(GetFName());
    FastNoiseWrapper = CreateDefaultSubobject<UFastNoiseWrapper>("FastNoiseWrapper");
    
    CollisionComponent->CoR = 0.95;
}

void ADynamicAsteroid::GenerateAsteroid()
{
    RandomStream.Reset();
    MeshData = FRealtimeMeshSimpleMeshData();
    URealtimeMeshBlueprintFunctionLibrary::AppendCapsuleMesh
        ( MeshData
        , FTransform::Identity
        , RP_SizeParam
        , RP_SizeParam
        , std::max(4, static_cast<int32>(RP_MeshResolution * FMathd::Sqrt(RP_SizeParam) * 3.) / 4)
        , std::max(8, static_cast<int32>(RP_MeshResolution * FMathd::Sqrt(RP_SizeParam) * 3.) / 2)
        , std::max(2, static_cast<int32>(RP_MeshResolution * FMathd::Sqrt(RP_SizeParam) * 3.) / 4)
        );
    
    NoiseSeed = RandomStream.GetUnsignedInt();
    FastNoiseWrapper->SetupFastNoise
        ( NoiseType
        , NoiseSeed
        , FrequencyFactor / RP_SizeParam
        , Interp
        , FractalType
        , Octaves
        , Lacunarity
        , Gain
        );

    // inertia calculation
    FVector SpatialDistributionSquared = FVector::Zero();
    check(MeshData.Positions.Num() == MeshData.Normals.Num())
    for(int i = 0; i < MeshData.Positions.Num(); i++)
    {
        const FVector Pos = MeshData.Positions[i];
        const float V = FastNoiseWrapper->GetNoise3D(Pos.X, Pos.Y, Pos.Z);
        const FVector NewPos = MeshData.Positions[i] + AmplitudeFactor * RP_SizeParam * V * MeshData.Normals[i];
        MeshData.Positions[i] = NewPos;
        SpatialDistributionSquared += FVector(pow(NewPos.X, 2), pow(NewPos.Y, 2), pow(NewPos.Z,2));
    }

    if(RP_bRecomputeNormals)
    {
        TArray<FProcMeshTangent> ProcTangents;
        UKismetProceduralMeshLibrary::CalculateTangentsForMesh
            ( MeshData.Positions
            , MeshData.Triangles
            , MeshData.UV0
            , MeshData.Normals
            , ProcTangents
            );
        for(int32 i = 0; i < ProcTangents.Num(); ++i)
        {
            MeshData.Tangents[i] = ProcTangents[i].TangentX;
            MeshData.Binormals[i] = MeshData.Normals[i].Cross(ProcTangents[i].TangentX);
        }
    }
    RotInertiaNorm = SpatialDistributionSquared.GetUnsafeNormal();
}

void ADynamicAsteroid::Fracture()
{
    for(auto [ FractureMeshData, VecOrigin] : GetFractures())
    {
        FActorSpawnParameters SpawnParameters;
        // Owner is "primarily used for replication"; I use it to have the spawned asteroid
        // added to the children array for destruction
        SpawnParameters.Owner = this;
        SpawnParameters.CustomPreSpawnInitalization = [this, FractureMeshData] (AActor* Actor)
        {
            // TODO: replace by some random distribution
            auto* DynamicAsteroid = Cast<ADynamicAsteroid>(Actor);
            DynamicAsteroid->SetMeshData(FractureMeshData);
            DynamicAsteroid->GetRealtimeMeshComponent()->SetMaterial
                (0, GetRealtimeMeshComponent()->GetMaterial(0));
            DynamicAsteroid->SetInitialOrbitParams
                ( { FVector(0.0, 0.0, 0.)
                    // TODO: only correct for VecR.Z == 0 
                , FVector(0., 0., 1.)
                });
#if WITH_EDITOR
            Actor->SetFolderPath(*GetName());
#endif
        };
        GetWorld()->SpawnActor<ADynamicAsteroid>
            ( this->GetClass()
            , GetActorLocation() + VecOrigin
            , FRotator::ZeroRotator
            , SpawnParameters
            );
    }
    this->Destroy();
}

TArray<FFractureInfo> ADynamicAsteroid::GetFractures()
{
    // try a simple z-plane-cut
    struct NewPosition
    {
        int32 FractureIndex;
        int32 VertexIndex;
    };
    TArray<NewPosition> PositionsMap;
    PositionsMap.SetNum(MeshData.Positions.Num(), false);

    FRealtimeMeshSimpleMeshData Fracture0, Fracture1;
    FVector Origin0 = FVector::Zero(), Origin1 = FVector::Zero();
    
    check(MeshData.Positions.Num() == MeshData.Normals.Num())
    check(MeshData.Positions.Num() == MeshData.Tangents.Num())
    for(int i = 0; i < MeshData.Positions.Num(); i++)
    {
        FVector Pos = MeshData.Positions[i];
        if(Pos.Z > 0)
        {
            int32 j = Fracture0.Positions.Add(Pos);
            Fracture0.Normals.Add(MeshData.Normals[i]);
            Fracture0.Tangents.Add((MeshData.Tangents[i]));
            PositionsMap[i] = { 0, j};
            Origin0 += Pos;
        }
        else
        {
            int32 j = Fracture1.Positions.Add(Pos);
            Fracture1.Normals.Add(MeshData.Normals[i]);
            Fracture1.Tangents.Add((MeshData.Tangents[i]));
            PositionsMap[i] = { 1, j };
            Origin1 += Pos;
        }
    }
    Origin0 /= Fracture0.Positions.Num();
    for(auto& Pos : Fracture0.Positions)
    {
        Pos -= Origin0;
    }
    Origin1 /= Fracture1.Positions.Num();
    for(auto& Pos : Fracture1.Positions)
    {
        Pos -= Origin1;
    }
    
    for(int i = 0; i < MeshData.Triangles.Num(); i += 3)
    {
        NewPosition NewPos1 = PositionsMap[MeshData.Triangles[i]];
        NewPosition NewPos2 = PositionsMap[MeshData.Triangles[i + 1]];
        NewPosition NewPos3 = PositionsMap[MeshData.Triangles[i + 2]];
        if(0 == NewPos1.FractureIndex == NewPos2.FractureIndex == NewPos3.FractureIndex)
        {
            Fracture0.Triangles.Add(NewPos1.VertexIndex);
            Fracture0.Triangles.Add(NewPos2.VertexIndex);
            Fracture0.Triangles.Add(NewPos3.VertexIndex);
        }
        else if(1 == NewPos1.FractureIndex == NewPos2.FractureIndex == NewPos3.FractureIndex)
        {
            Fracture1.Triangles.Add(NewPos1.VertexIndex);
            Fracture1.Triangles.Add(NewPos2.VertexIndex);
            Fracture1.Triangles.Add(NewPos3.VertexIndex);
        }
        // else: the triangle gets discarded; better would be to properly cut it and create new vertices
    }
    check(Fracture0.Triangles.Num() % 3 == 0)
    check(Fracture1.Triangles.Num() % 3 == 0)

    return
        { { Fracture0, Origin0 }
        , { Fracture1, Origin1 }
        };
}

FVector ADynamicAsteroid::GetInitialOmega()
{
    check(IsValid(CurveOmegaDistribution))
    return OmegaMax * CurveOmegaDistribution->GetFloatValue(RandomStream.FRand()) * FVector::UnitZ();
}

void ADynamicAsteroid::OnGenerateMesh_Implementation()
{
    Super::OnGenerateMesh_Implementation();

    auto* RealtimeMesh = GetRealtimeMeshComponent()->InitializeRealtimeMesh<URealtimeMeshSimple>();
    RealtimeMesh->SetupMaterialSlot(0, "Material");

    if(AsteroidTypes.IsEmpty())
    {
        UE_LOGFMT(LogMyGame, Warning, "{0}: AsteroidTypes empty", GetFName());
    }
    else
    {
        for(auto AsteroidType : AsteroidTypes)
        {
            if(!IsValid(AsteroidType.Material))
            {
                UE_LOGFMT(LogMyGame, Warning, "{0}: Invalid material in AsteroidTypes", GetFName());
            }
        }
        auto [MaterialInstance, CoR] = SelectAsteroidType();
        RealtimeMeshComponent->SetMaterial(0, MaterialInstance);
    }

    switch(Origin)
    {
    case EDynamicAsteroidOrigin::SelfGenerated:
        {
            GenerateAsteroid();
        
            FRealtimeMeshCollisionConfiguration CollisionConfiguration;
            CollisionConfiguration.bUseComplexAsSimpleCollision = false;
            RealtimeMesh->SetCollisionConfig(CollisionConfiguration);

            FRealtimeMeshCollisionCapsule CollisionCapsule;
            CollisionCapsule.Radius = RP_SizeParam;
            CollisionCapsule.Length = RP_SizeParam;
            int32 CCIndex;
            FRealtimeMeshSimpleGeometry Geo;
            URealtimeMeshSimpleGeometryFunctionLibrary::AddCapsule (Geo , CollisionCapsule , CCIndex);
            RealtimeMesh->SetSimpleGeometry(Geo);
            RealtimeMesh->UpdateCollision(true);
            auto* BodySetup = RealtimeMesh->GetBodySetup();
            Geo.CopyToBodySetup(BodySetup);

            // auto& BodyCapsule = GetRealtimeMeshComponent()->GetBodySetup()->AggGeom.SphylElems.AddDefaulted_GetRef();
            // BodyCapsule.Radius = SizeParam;
            // BodyCapsule.Length = SizeParam;
            // BodyCapsule.SetContributeToMass(true);
            // BodyCapsule.SetName("CollisionCapsule");
            break;
        }
        
    case EDynamicAsteroidOrigin::FromMeshData:
        break;
    }
    
    RealtimeMesh->CreateMeshSection
        (0
        , // TODO: Section Draw Type: test Static/Dynamic performance
          FRealtimeMeshSectionConfig(ERealtimeMeshSectionDrawType::Static, 0)
        , MeshData
        , false
        );

    // TODO: set based on asteroid type
    CollisionComponent->Density = 2.;
    CollisionComponent->DensityExponent = 2.9;
    CollisionComponent->UpdateMass(GetBounds().SphereRadius);
}

void ADynamicAsteroid::Destroyed()
{
    Super::Destroyed();
    if(IsValid(AsteroidBelt))
    {
        AsteroidBelt->ClearAsteroidPointer(this);
    }
}

void ADynamicAsteroid::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    
#if WITH_EDITOR
    if(IsValid(AsteroidBelt))
    {
        // too many white lines for splines in asteroid belts in the editor
        RP_Orbit->SetDrawDebug(false);
        RP_Orbit->SetFolderPath(*AsteroidBelt->GetName());
    }
#endif
}

void ADynamicAsteroid::BeginPlay()
{
    Super::BeginPlay();
    OnGenerateMesh();
}

FSelectedAsteroidType ADynamicAsteroid::SelectAsteroidType()
{
    TArray<FSelectedAsteroidType> ValidAsteroidTypes;
    for(auto [Material, MinSize, MaxSize, CoR] : AsteroidTypes)
    {
        if(RP_SizeParam >= MinSize && RP_SizeParam <= MaxSize)
        {
            const double Variation = RandomStream.FRand() * 0.2 - 0.1;
            const double CoRVaried = std::max(0., CoR + Variation);
            ValidAsteroidTypes.Push({Material, CoRVaried});
        }
    }
    if(ValidAsteroidTypes.IsEmpty())
    {
        UE_LOGFMT(LogMyGame, Warning, "{0}: No valid material in Materials", GetFName());
        return {};
    }
    return ValidAsteroidTypes[RandomStream.RandRange(0, ValidAsteroidTypes.Num() - 1)];
}

void ADynamicAsteroid::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(ADynamicAsteroid, RP_Seed, COND_InitialOnly)
    DOREPLIFETIME_CONDITION(ADynamicAsteroid, RP_MeshResolution, COND_InitialOnly)
    DOREPLIFETIME_CONDITION(ADynamicAsteroid, RP_bRecomputeNormals, COND_InitialOnly)
    DOREPLIFETIME_CONDITION(ADynamicAsteroid, RP_SizeParam, COND_InitialOnly)
}