#include "Actors/DynamicAsteroid.h"

#include "RealtimeMeshLibrary.h"
#include "RealtimeMeshSimple.h"
#include "../../../../Plugins/SimplexNoise/Source/SimplexNoise/Public/SimplexNoiseBPLibrary.h"
#include "GeometryScript/MeshNormalsFunctions.h"

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
		, SizeParam
		, SizeParam
		, std::max(4, static_cast<int32>(SizeParam) / 20)
		, std::max(8, static_cast<int32>(SizeParam) / 10)
		, std::max(2, static_cast<int32>(SizeParam) / 20)
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
		//V = USimplexNoiseBPLibrary::SimplexNoise3D(Pos.X, Pos.Y, Pos.Z, SxFrequencyFactor / SizeParam);
		MeshData.Positions[i] += SxAmplitudeFactor * SizeParam * V * MeshData.Normals[i];
	}
	// UGeometryScriptLibrary_MeshNormalsFunctions::RecomputeNormals()
	// UGeometryScriptLibrary_MeshNormalsFunctions::AutoRepairNormals()
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
