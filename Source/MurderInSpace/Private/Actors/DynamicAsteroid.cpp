#include "Actors/DynamicAsteroid.h"

#include "RealtimeMeshLibrary.h"
#include "RealtimeMeshSimple.h"

ADynamicAsteroid::ADynamicAsteroid()
{
}

void ADynamicAsteroid::GenerateMesh(FRandomStream RandomStream, double SizeParam)
{
}

void ADynamicAsteroid::OnGenerateMesh_Implementation()
{
	// Initialize the simple mesh
	auto* RealtimeMesh = GetRealtimeMeshComponent()->InitializeRealtimeMesh<URealtimeMeshSimple>();

	// This example create 3 rectangular prisms, one on each axis, with 2 of them grouped in the same vertex buffers, but with different sections
	// This allows for setting up separate materials even if sections share a single set of buffers.
	// Here we do a latent mesh submission, so we create the mesh section group and sections first, and then apply the mesh data later
	
	FRealtimeMeshSimpleMeshData MeshData;
	
	// Create a single section, with its own dedicated section group
	FRealtimeMeshSectionKey StaticSectionKey = RealtimeMesh->CreateMeshSection
		(0
		, FRealtimeMeshSectionConfig(ERealtimeMeshSectionDrawType::Static, 0)
		, MeshData
		, true
		);
	// Create a section group passing it our mesh data
	FRealtimeMeshSectionGroupKey GroupKey = RealtimeMesh->CreateSectionGroupWithMesh(0, MeshData);

	// Create both sections on the same mesh data
	FRealtimeMeshSectionKey SectionA = RealtimeMesh->CreateSectionInGroup
		(GroupKey
		, FRealtimeMeshSectionConfig(ERealtimeMeshSectionDrawType::Static, 0)
		, FRealtimeMeshStreamRange()
		, true
		);
	FRealtimeMeshSectionKey SectionB = RealtimeMesh->CreateSectionInGroup
		( GroupKey
		, FRealtimeMeshSectionConfig(ERealtimeMeshSectionDrawType::Static, 0)
		, FRealtimeMeshStreamRange()
		, true
		);
	
	{	// Create a basic single section
		FRealtimeMeshSimpleMeshData BoxMesh;

		// This just adds a simple box, you can instead create your own mesh data
		URealtimeMeshBlueprintFunctionLibrary::AppendBoxMesh(FVector(100, 100, 200), FTransform::Identity, BoxMesh);

		RealtimeMesh->UpdateSectionMesh(StaticSectionKey, BoxMesh);

	}
	
	{
		FRealtimeMeshSimpleMeshData BoxMesh;

		URealtimeMeshBlueprintFunctionLibrary::AppendBoxMesh(FVector(200, 100, 100), FTransform::Identity, BoxMesh);
		URealtimeMeshBlueprintFunctionLibrary::AppendBoxMesh(FVector(100, 200, 100), FTransform::Identity, BoxMesh);

		RealtimeMesh->UpdateSectionGroupMesh(GroupKey, BoxMesh);
		RealtimeMesh->UpdateSectionSegment(SectionA, FRealtimeMeshStreamRange(0, 24, 0, 36));
		RealtimeMesh->UpdateSectionSegment(SectionB, FRealtimeMeshStreamRange(24, 48, 36, 72));
	}
	
	check(!MaterialTypes.IsEmpty())
	for(auto MaterialType : MaterialTypes)
	{
		check(IsValid(MaterialType.Material))
	}

	// TODO: find out how to pass params to mesh generation
	const float SizeParam = 0;
    //auto* DynamicMesh = RealtimeMeshComponent->GetDynamicMesh();
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
	// TODO for RMC
    // UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendCapsule
    //     ( DynamicMesh
    //     , GeometryScriptPrimitiveOptions
    //     , Transform
    //     , Radius
    //     , LineLength
    //     , HemisphereSteps
    //     , CircleSteps
    //     , EGeometryScriptPrimitiveOriginMode::Center
    //     );

	// TODO: collision capsule
	// int32 FRealtimeMeshSimpleGeometry::AddCapsule(const FRealtimeMeshCollisionCapsule& InCapsule)

	// FKSphylElem Capsule;
	// Capsule.Radius = CollisionCapsuleRelativeSize * Radius;
	// Capsule.Length = CollisionCapsuleRelativeSize * LineLength;
	// auto* BodySetup = StaticMesh->GetBodySetup();
    // BodySetup->AggGeom.SphylElems.Add(Capsule);
	// BodySetup->CreatePhysicsMeshes();

	// TODO: RandomStream param
	RealtimeMeshComponent->SetMaterial(0, SelectMaterial(FRandomStream(), SizeParam));
	Super::OnGenerateMesh_Implementation();
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
