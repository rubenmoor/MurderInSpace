#include "Actors/MyActor_StaticMesh.h"

#include "PlanarCut.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "GeometryCollection/GeometryCollectionEngineConversion.h"
#include "GeometryCollection/GeometryCollectionObject.h"
#include "MyComponents/GyrationComponent.h"
#include "MyComponents/MyCollisionComponent.h"
#include "Net/UnrealNetwork.h"
#include "PlanarCut/Private/GeometryMeshConversion.h"
#include "Voronoi/Voronoi.h"

AMyActor_StaticMesh::AMyActor_StaticMesh()
{
    bNetLoadOnClient = false;
    bReplicates = true;
    bAlwaysRelevant = true;
    AActor::SetReplicateMovement(false);
    
    Root = CreateDefaultSubobject<USceneComponent>("Root");
    SetRootComponent(Root);
    
    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
    StaticMesh->SetupAttachment(Root);

    GeometryCollection = CreateDefaultSubobject<UGeometryCollectionComponent>("GeometryCollection");
    GeometryCollection->SetupAttachment(Root);
    GeometryCollection->SetSimulatePhysics(false);
    GeometryCollection->SetVisibility(false);
    
    Gyration = CreateDefaultSubobject<UGyrationComponent>("Gyration");
    Collision = CreateDefaultSubobject<UMyCollisionComponent>("Collision");
}

void AMyActor_StaticMesh::BeginPlay()
{
    Super::BeginPlay();
    GeometryCollection->SetVisibility(false);
}

void AMyActor_StaticMesh::Destroyed()
{
    Super::Destroyed();
    if(IsValid(RP_Orbit))
    {
        RP_Orbit->Destroy();
    }
}

void AMyActor_StaticMesh::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    ensureMsgf(GetLocalRole() == ROLE_Authority, TEXT("%s: OnConstruction while Role < Authority"), *GetFullName());
    
    if  (
        GetWorld()->WorldType != EWorldType::EditorPreview
        
        // avoid orbit spawning when dragging an actor with orbit into the viewport at first
        // The preview actor that is created doesn't have a valid location
        // Once the actor is placed inside the viewport, it's no longer transient and the orbit is reconstructed properly
        // according to the actor location
        && !HasAnyFlags(RF_Transient)
        )
    {
        OrbitSetup(this);
    }

    MyMass = pow(StaticMesh->Bounds.SphereRadius, 3);
    
    // setup geometry collection component
    // TODO: check if world check is necessary
    //if(GetWorld()->WorldType != EWorldType::EditorPreview)

    if(!bGeometryCollectionSetupDone)
    {
        const FSoftObjectPath SourcePath(StaticMesh->GetStaticMesh());
        TArray<TObjectPtr<UMaterialInterface>> SourceMaterials(StaticMesh->GetMaterials());
        TWeakObjectPtr<UGeometryCollection> RestCollection =
            NewObject<UGeometryCollection>(GetWorld(), UGeometryCollection::StaticClass());
        RestCollection->SetFlags(RF_Transactional | RF_Public | RF_Standalone);
        if(RestCollection->SizeSpecificData.IsEmpty())
        {
            RestCollection->SizeSpecificData.Add(FGeometryCollectionSizeSpecificData());
        }
        GeometryCollection->SetRestCollection(RestCollection.Get());
        FTransform ComponentTransform(StaticMesh->GetComponentTransform());
        ComponentTransform.SetTranslation(ComponentTransform.GetTranslation() - GetTransform().GetTranslation());
        RestCollection->GeometrySource.Add(
            { SourcePath
            , ComponentTransform
            , SourceMaterials
            , true
            , false 
            });
        FGeometryCollectionEngineConversion::AppendStaticMesh
            ( StaticMesh->GetStaticMesh()
            , SourceMaterials
            , ComponentTransform
            , RestCollection.Get()
            , false
            , true
            , false
            );
        RestCollection->InitializeMaterials();
        // TODO: check if necessary
        //UFractureActionTool::AddSingleRootNodeIfRequired
        GeometryCollection->MarkPackageDirty();
        GeometryCollection->MarkRenderStateDirty();
        //::GeometryCollection::GenerateTemporaryGuids
        //	( FracturedGeometryCollection->GetGeometryCollection().Get()
        //	, 0, true);
        //FGeometryCollectionProximityUtility ProximityUtility(FracturedGeometryCollection->GetGeometryCollection().Get());
        //ProximityUtility.UpdateProximity();

        
        TArray<FVector> Sites;

        int32 NumberVoronoiSitesMin = 20;
        int32 NumberVoronoiSitesMax = 20;
        FRandomStream RandStream(-1);

        //FBox Bounds = Context.GetWorldBounds();
        FVector BoundsMin, BoundsMax;
        StaticMesh->GetLocalBounds(BoundsMin, BoundsMax);
        const FVector Extent(BoundsMax - BoundsMin);

        const int32 SiteCount = RandStream.RandRange(NumberVoronoiSitesMin, NumberVoronoiSitesMax);

        Sites.Reserve(Sites.Num() + SiteCount);
        for (int32 ii = 0; ii < SiteCount; ++ii)
        {
            Sites.Emplace(BoundsMin + FVector(RandStream.FRand(), RandStream.FRand(), RandStream.FRand()) * Extent );
        }
        
        // FBox VoronoiBounds = FractureContext.GetWorldBounds(); 
        // if (Sites.Num() > 0)
        // {
        // 	VoronoiBounds += FBox(Sites);
        // }
        // 
        // return VoronoiBounds.ExpandBy(CutterSettings->GetMaxVertexMovement() + KINDA_SMALL_NUMBER);
        FBox VoronoiBounds = FBox(Sites).ExpandBy(1e-3);
        
        //VoronoiOp->Selection = FractureContext.GetSelection();
        //VoronoiOp->Grout = CutterSettings->Grout;
        //VoronoiOp->PointSpacing = CollisionSettings->GetPointSpacing();
        //VoronoiOp->Sites = Sites;
        //if (CutterSettings->Amplitude > 0.0f)
        //{
        //	FNoiseSettings Settings;
        //	CutterSettings->TransferNoiseSettings(Settings);
        //	VoronoiOp->NoiseSettings = Settings;
        //}
        //VoronoiOp->Seed = FractureContext.GetSeed();
        //VoronoiOp->Transform = FractureContext.GetTransform();

        FVoronoiDiagram Voronoi(Sites, VoronoiBounds, .1f);
        FPlanarCells VoronoiPlanarCells = FPlanarCells(Sites, Voronoi);
        FNoiseSettings NoiseSettings;
        // TODO: NoiseSettings
        VoronoiPlanarCells.InternalSurfaceMaterials.NoiseSettings = NoiseSettings;

        TUniquePtr<FGeometryCollection> CollectionCopy = MakeUnique<FGeometryCollection>();
        CollectionCopy->CopyMatchingAttributesFrom(*RestCollection->GetGeometryCollection(), nullptr);
        VoronoiPlanarCells.InternalSurfaceMaterials.SetUVScaleFromCollection(*CollectionCopy);

        FVector Origin = ComponentTransform.GetTranslation();
        FTransform CollectionToWorld(ComponentTransform * FTransform(-Origin));
        //const TArrayView<const int32>& TransformIndices;
        //UE::PlanarCut::FDynamicMeshCollection DynamicMeshCollection(CollectionCopy, TransformIndices, CollectionToWorld);

        GeometryCollection->SetVisibility(false);
        
        bGeometryCollectionSetupDone = true;
    }
}

void AMyActor_StaticMesh::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(AMyActor_StaticMesh, RP_Orbit      , COND_InitialOnly)
}

#if WITH_EDITOR
void AMyActor_StaticMesh::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
    Super::PostEditChangeChainProperty(PropertyChangedEvent);

    const FName Name = PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName();

    static const FName FNameOrbitColor = GET_MEMBER_NAME_CHECKED(AMyActor_StaticMesh, OrbitColor);

    if(Name == FNameOrbitColor)
    {
        if(IsValid(RP_Orbit))
        {
            RP_Orbit->Update(PhysicsEditorDefault, InstanceUIEditorDefault);
        }
    }
}
#endif
