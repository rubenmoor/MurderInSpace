#include "Actors/DynamicAsteroid.h"

#include "DynamicMeshToMeshDescription.h"
#include "MeshDescription.h"
#include "StaticMeshAttributes.h"
#include "GeometryScript/MeshDeformFunctions.h"
#include "GeometryScript/MeshMaterialFunctions.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "GeometryScript/MeshUVFunctions.h"

ADynamicAsteroid::ADynamicAsteroid()
{
    DynamicMeshComponent = CreateDefaultSubobject<UDynamicMeshComponent>("DynamicMeshComponent");
    DynamicMeshComponent->SetupAttachment(Root);
	DynamicMeshComponent->PrimaryComponentTick.bCanEverTick = false;

	StaticMeshComponent->bDrawMeshCollisionIfSimple = true;
	StaticMeshComponent->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	StaticMeshComponent->SetGenerateOverlapEvents(false);
}

void ADynamicAsteroid::GenerateMesh(FRandomStream RandomStream, double SizeParam)
{
	check(!MaterialTypes.IsEmpty())
	for(auto MaterialType : MaterialTypes)
	{
		check(IsValid(MaterialType.Material))
	}

    auto* DynamicMesh = DynamicMeshComponent->GetDynamicMesh();
    DynamicMesh->Reset();
    const FGeometryScriptPrimitiveOptions GeometryScriptPrimitiveOptions;
    const FTransform Transform;
    const float Radius = SizeParam;
    const float LineLength = SizeParam;
    const int32 HemisphereSteps = 15;
    const int32 CircleSteps = 12;
    UE_LOG(LogMyGame, Display
        , TEXT("%s: Generating capsule: Radius %.0f, LineLength %.0f, HemisphereSteps %d, CircleSteps %d")
        , *GetFullName()
        , Radius, LineLength, HemisphereSteps, CircleSteps
        )
    UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendCapsule
        ( DynamicMesh
        , GeometryScriptPrimitiveOptions
        , Transform
        , Radius
        , LineLength
        , HemisphereSteps
        , CircleSteps
        , EGeometryScriptPrimitiveOriginMode::Center
        );
	FGeometryScriptRecomputeUVsOptions GeometryScriptRecomputeUVsOptions;
	FGeometryScriptMeshSelection Selection;
	UGeometryScriptLibrary_MeshUVFunctions::RecomputeMeshUVs(DynamicMesh, 0, GeometryScriptRecomputeUVsOptions, Selection);
	FGeometryScriptPerlinNoiseOptions GeometryScriptPerlinNoiseOptions;
	UGeometryScriptLibrary_MeshDeformFunctions::ApplyPerlinNoiseToMesh(DynamicMesh, Selection, GeometryScriptPerlinNoiseOptions);
	//UGeometryScriptLibrary_MeshUVFunctions::CopyMeshToMeshUVLayer(DynamicMesh, 0, )

    // TODO: mass?
    // TODO: apply distortions based on asteroid type
    // TODO: inertia
    // probably layers of perlin noise
    
    auto* StaticMesh = NewObject<UStaticMesh>(GetWorld());

    FStaticMeshSourceModel& SrcModel = StaticMesh->AddSourceModel();
    SrcModel.BuildSettings.bRecomputeNormals = false;

    FMeshDescription MeshDescription;
    FStaticMeshAttributes StaticMeshAttributes(MeshDescription);
    StaticMeshAttributes.Register();

    const FDynamicMesh3* Mesh = DynamicMesh->GetMeshPtr();

    FDynamicMeshToMeshDescription().Convert(Mesh, MeshDescription);
    
    // Build the static mesh render data, one FMeshDescription* per LOD.
    UStaticMesh::FBuildMeshDescriptionsParams BuildMeshDescriptionsParams;
    //BuildMeshDescriptionsParams.bBuildSimpleCollision = true;
    // TODO: according to doc, "mandatory in non-editor builds", defaults to false: testing needed
    BuildMeshDescriptionsParams.bFastBuild = true;
    StaticMesh->BuildFromMeshDescriptions({&MeshDescription}, BuildMeshDescriptionsParams);
	FKSphylElem Capsule;
	Capsule.Radius = CollisionCapsuleRelativeSize * Radius;
	Capsule.Length = CollisionCapsuleRelativeSize * LineLength;
	auto* BodySetup = StaticMesh->GetBodySetup();
    BodySetup->AggGeom.SphylElems.Add(Capsule);
	BodySetup->CreatePhysicsMeshes();
	
		// if (Params.bBuildSimpleCollision)
		// {
		// 	FKBoxElem BoxElem;
		// 	BoxElem.Center = GetRenderData()->Bounds.Origin;
		// 	BoxElem.X = GetRenderData()->Bounds.BoxExtent.X * 2.0f;
		// 	BoxElem.Y = GetRenderData()->Bounds.BoxExtent.Y * 2.0f;
		// 	BoxElem.Z = GetRenderData()->Bounds.BoxExtent.Z * 2.0f;
		// 	GetBodySetup()->AggGeom.BoxElems.Add(BoxElem);
		// 	GetBodySetup()->CreatePhysicsMeshes();
		// }

    StaticMeshComponent->SetStaticMesh(StaticMesh);
    //StaticMesh->SetMaterial(0, SelectMaterial(RandomStream, SizeParam));
	StaticMeshComponent->SetMaterial(0, SelectMaterial(RandomStream, SizeParam));
	
    // TODO: benchmark
    //DynamicAsteroidMesh->SetVisibility(false);
    DynamicMeshComponent->DestroyComponent();
}

void ADynamicAsteroid::OnConstruction(const FTransform& Transform)
{
    // TODO: maybe proper preview asteroid
    // probably requires excluding editor-preview/transient
    // or just require IsValid(this)/IsValid(World)/IsValid(DynamicMeshComponent)
    Super::OnConstruction(Transform);
}

UMaterialInstance* ADynamicAsteroid::SelectMaterial(FRandomStream RandomStream, double SizeParam)
{
	TArray<UMaterialInstance*> Materials;
	for(auto [Material, MinSize, MaxSize] : MaterialTypes)
	{
		if(SizeParam > MinSize && SizeParam < MaxSize)
		{
			Materials.Push(Material);
		}
	}
	check(!Materials.IsEmpty())
	return Materials[RandomStream.RandRange(0, Materials.Num() - 1)];
}
