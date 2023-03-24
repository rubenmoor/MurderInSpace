#include "Actors/DynamicAsteroid.h"

#include "RealtimeMeshLibrary.h"
#include "RealtimeMeshSimple.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"

ADynamicAsteroid::ADynamicAsteroid()
{
}

void ADynamicAsteroid::OnGenerateMesh_Implementation()
{
	Super::OnGenerateMesh_Implementation();

	// TODO: how to pass parameters here?
	// maybe: pre spawn initialization by asteroid belt
	// or:    pull parameters from GetOwner()

	auto* RealtimeMesh = GetRealtimeMeshComponent()->InitializeRealtimeMesh<URealtimeMeshSimple>();
	RealtimeMesh->SetupMaterialSlot(0, "Material");

	for(auto MaterialType : MaterialTypes)
	{
		if(!IsValid(MaterialType.Material))
		{
			UE_LOG(LogMyGame, Warning, TEXT("%s: Invalid material in MaterialTypes"), *GetFullName())
		}
	}

	FRealtimeMeshSimpleMeshData MeshData;
	URealtimeMeshBlueprintFunctionLibrary::AppendCapsuleMesh
		( MeshData
		, FTransform::Identity
		, SizeParam / 2.
		, SizeParam
		, std::max(4, static_cast<int32>(SizeParam) / 50)
		, std::max(8, static_cast<int32>(SizeParam) / 25)
		, std::max(2, static_cast<int32>(SizeParam) / 50)
		);
	RealtimeMesh->CreateMeshSection
		(0
		, // TODO: Section Draw Type: test Static/Dynamic performance
		  FRealtimeMeshSectionConfig(ERealtimeMeshSectionDrawType::Static, 0)
		, MeshData
		, false
		);
	
	// TODO: collision capsule
	// how to add?
	//int32 FRealtimeMeshSimpleGeometry::AddCapsule(const FRealtimeMeshCollisionCapsule& CollisionCapsule);

	if(MaterialTypes.IsEmpty())
	{
		UE_LOG(LogMyGame, Warning, TEXT("%s: MaterialTypes empty"), *GetFullName())
	}
	else
	{
		RealtimeMeshComponent->SetMaterial(0, SelectMaterial());
	}
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
