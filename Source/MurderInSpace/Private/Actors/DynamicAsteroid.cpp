#include "Actors/DynamicAsteroid.h"

#include "KismetProceduralMeshLibrary.h"
#include "RealtimeMesh.h"
#include "RealtimeMeshLibrary.h"
#include "RealtimeMeshSimple.h"
#include "../../../../Plugins/SimplexNoise/Source/SimplexNoise/Public/SimplexNoiseBPLibrary.h"

ADynamicAsteroid::ADynamicAsteroid()
{
}

void ADynamicAsteroid::GenerateAsteroid()
{
    MeshData = FRealtimeMeshSimpleMeshData();
    URealtimeMeshBlueprintFunctionLibrary::AppendCapsuleMesh
        ( MeshData
        , FTransform::Identity
        , SizeParam
        , SizeParam
        , std::max(4, static_cast<int32>(MeshResolution * FMathd::Sqrt(SizeParam) * 3.) / 4)
        , std::max(8, static_cast<int32>(MeshResolution * FMathd::Sqrt(SizeParam) * 3) / 2)
        , std::max(2, static_cast<int32>(MeshResolution * FMathd::Sqrt(SizeParam) * 3) / 4)
        );

    USimplexNoiseBPLibrary::setNoiseFromStream(RandomStream);
    check(MeshData.Positions.Num() == MeshData.Normals.Num())
    for(int i = 0; i < MeshData.Positions.Num(); i++)
    {
        auto Pos = MeshData.Positions[i];
        float V = USimplexNoiseBPLibrary::GetSimplexNoise3D_EX
            ( Pos.X, Pos.Y, Pos.Z
            , SxLacunarity
            , SxPersistance
            , SxOctaves
            , SxFrequencyFactor / SizeParam
            , false
            );
        MeshData.Positions[i] += SxAmplitudeFactor * SizeParam * V * MeshData.Normals[i];
    }

    if(bRecomputeNormals)
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
    
    if(MaterialTypes.IsEmpty())
    {
        UE_LOG(LogMyGame, Warning, TEXT("%s: MaterialTypes empty"), *GetFullName())
    }
    else
    {
        RealtimeMeshComponent->SetMaterial(0, SelectMaterial());
    }
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

void ADynamicAsteroid::OnGenerateMesh_Implementation()
{
    Super::OnGenerateMesh_Implementation();

    auto* RealtimeMesh = GetRealtimeMeshComponent()->InitializeRealtimeMesh<URealtimeMeshSimple>();
    if(!IsValid(RealtimeMesh))
    {
        UE_LOG(LogMyGame, Warning, TEXT("%s: OnGenerateMesh: GetRealtimeMesh(): invalid"), *GetFullName())
        return;
    }
    RealtimeMesh->SetupMaterialSlot(0, "Material");

    for(auto MaterialType : MaterialTypes)
    {
        if(!IsValid(MaterialType.Material))
        {
            UE_LOG(LogMyGame, Warning, TEXT("%s: Invalid material in MaterialTypes"), *GetFullName())
        }
    }

    if(MeshData.Positions.IsEmpty())
    {
        GenerateAsteroid();
        
        FRealtimeMeshCollisionConfiguration CollisionConfiguration;
        CollisionConfiguration.bUseComplexAsSimpleCollision = false;
        RealtimeMesh->SetCollisionConfig(CollisionConfiguration);
        
        FRealtimeMeshCollisionCapsule CollisionCapsule;
        CollisionCapsule.Radius = SizeParam;
        CollisionCapsule.Length = SizeParam;
        int32 CCIndex;
        auto Geo = RealtimeMesh->GetSimpleGeometry();
        URealtimeMeshSimpleGeometryFunctionLibrary::AddCapsule (Geo , CollisionCapsule , CCIndex);
        RealtimeMesh->UpdateCollision();
        Geo.CopyToBodySetup(RealtimeMesh->GetBodySetup());
        
        // auto& BodyCapsule = GetRealtimeMeshComponent()->GetBodySetup()->AggGeom.SphylElems.AddDefaulted_GetRef();
        // BodyCapsule.Radius = SizeParam;
        // BodyCapsule.Length = SizeParam;
        // BodyCapsule.SetContributeToMass(true);
        // BodyCapsule.SetName("CollisionCapsule");
        
    }
    
    RealtimeMesh->CreateMeshSection
        (0
        , // TODO: Section Draw Type: test Static/Dynamic performance
          FRealtimeMeshSectionConfig(ERealtimeMeshSectionDrawType::Static, 0)
        , MeshData
        , false
        );
    
    Super::OnGenerateMesh_Implementation();
}

UMaterialInstance* ADynamicAsteroid::SelectMaterial()
{
    TArray<UMaterialInstance*> Materials;
    for(auto [Material, MinSize, MaxSize] : MaterialTypes)
    {
        if(SizeParam >= MinSize && SizeParam <= MaxSize)
        {
            Materials.Push(Material);
        }
    }
    if(Materials.IsEmpty())
    {
        UE_LOG(LogMyGame, Warning, TEXT("%s: No valid material in Materials"), *GetFullName())
        return nullptr;
    }
    return Materials[RandomStream.RandRange(0, Materials.Num() - 1)];
}
