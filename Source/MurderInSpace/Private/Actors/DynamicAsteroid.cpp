#include "Actors/DynamicAsteroid.h"

#include "RealtimeMeshLibrary.h"
#include "RealtimeMeshSimple.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"

ADynamicAsteroid::ADynamicAsteroid()
{
}

void ADynamicAsteroid::GenerateMesh(FRandomStream RandomStream, double SizeParam)
{
}

void ADynamicAsteroid::OnGenerateMesh_Implementation()
{
	Super::OnGenerateMesh_Implementation();

	auto* RealtimeMesh = GetRealtimeMeshComponent()->InitializeRealtimeMesh<URealtimeMeshSimple>();
	RealtimeMesh->SetupMaterialSlot(0, "Material");

	{	// Create a basic single section
		FRealtimeMeshSimpleMeshData MeshData;

		// This just adds a simple box, you can instead create your own mesh data
		URealtimeMeshBlueprintFunctionLibrary::AppendBoxMesh(FVector(100, 100, 200), FTransform::Identity, MeshData);
		URealtimeMeshBlueprintFunctionLibrary::AppendMesh()

		// Create a single section, with its own dedicated section group
		FRealtimeMeshSectionKey StaticSectionKey = RealtimeMesh->CreateMeshSection(0,
			FRealtimeMeshSectionConfig(ERealtimeMeshSectionDrawType::Static, 0), MeshData, true);
	}
	
	for(auto MaterialType : MaterialTypes)
	{
		if(!IsValid(MaterialType.Material))
		{
			UE_LOG(LogMyGame, Warning, TEXT("%s: Invalid material in MaterialTypes"), *GetFullName())
		}
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
    //UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendCapsule
    //    ( DynamicMesh
    //    , GeometryScriptPrimitiveOptions
    //    , Transform
    //    , Radius
    //    , LineLength
    //    , HemisphereSteps
    //    , CircleSteps
    //    , EGeometryScriptPrimitiveOriginMode::Center
    //    );

	// TODO: collision capsule
	// int32 FRealtimeMeshSimpleGeometry::AddCapsule(const FRealtimeMeshCollisionCapsule& InCapsule)

	// FKSphylElem Capsule;
	// Capsule.Radius = CollisionCapsuleRelativeSize * Radius;
	// Capsule.Length = CollisionCapsuleRelativeSize * LineLength;
	// auto* BodySetup = StaticMesh->GetBodySetup();
    // BodySetup->AggGeom.SphylElems.Add(Capsule);
	// BodySetup->CreatePhysicsMeshes();

	if(MaterialTypes.IsEmpty())
	{
		UE_LOG(LogMyGame, Warning, TEXT("%s: MaterialTypes empty"), *GetFullName())
	}
	else
	{
	// TODO: RandomStream param
		RealtimeMeshComponent->SetMaterial(0, SelectMaterial(FRandomStream(), SizeParam));
	}
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
