﻿#include "Actors/DynamicAsteroid.h"

#include "DynamicMeshToMeshDescription.h"
#include "MeshDescription.h"
#include "StaticMeshAttributes.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"

ADynamicAsteroid::ADynamicAsteroid()
{
    DynamicMeshComponent = CreateDefaultSubobject<UDynamicMeshComponent>("DynamicMesh");
    DynamicMeshComponent->SetupAttachment(Root);
}

void ADynamicAsteroid::GenerateMesh(float SizeParam)
{
    auto* DynamicMesh = DynamicMeshComponent->GetDynamicMesh();
    DynamicMesh->Reset();
    const FGeometryScriptPrimitiveOptions GeometryScriptPrimitiveOptions;
    const FTransform Transform;
    const float Radius = SizeParam;
    const float LineLength = SizeParam;
    const int32 HemisphereSteps = 5;
    const int32 CircleSteps = 8;
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

    // TODO: texture/material
    // TODO: mass?
    // TODO: apply distortions based on asteroid type
    // TODO: inertia
    // probably layers of perlin noise
    
    UStaticMesh* StaticMesh = NewObject<UStaticMesh>(GetWorld());

    FStaticMeshSourceModel& SrcModel = StaticMesh->AddSourceModel();
    SrcModel.BuildSettings.bRecomputeNormals = false;

    FMeshDescription MeshDescription;
    FStaticMeshAttributes StaticMeshAttributes(MeshDescription);
    StaticMeshAttributes.Register();

    const FDynamicMesh3* Mesh = DynamicMesh->GetMeshPtr();

    FDynamicMeshToMeshDescription Converter;
    Converter.Convert(Mesh, MeshDescription);
    
    // Build the static mesh render data, one FMeshDescription* per LOD.
    StaticMesh->BuildFromMeshDescriptions({&MeshDescription});

    // TODO: capsule collision
    StaticMeshComponent->SetStaticMesh(StaticMesh);
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