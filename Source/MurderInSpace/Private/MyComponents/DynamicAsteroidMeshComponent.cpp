// Fill out your copyright notice in the Description page of Project Settings.


#include "MyComponents/DynamicAsteroidMeshComponent.h"
#include "StaticMeshAttributes.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "DynamicMeshToMeshDescription.h"
#include "MeshDescription.h"

UStaticMesh* UDynamicAsteroidMeshComponent::MakeStaticMesh()
{
    GetDynamicMesh()->Reset();
    FGeometryScriptPrimitiveOptions GeometryScriptPrimitiveOptions;
    FTransform Transform;
    UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendCapsule
        ( GetDynamicMesh()
        , GeometryScriptPrimitiveOptions
        , Transform
        );

    // TODO: texture/material
    // TODO: mass?
    // TODO: apply distortions based on asteroid type
    // probably layers of perlin noise
    
    UStaticMesh* StaticMesh = NewObject<UStaticMesh>(GetWorld());

    FStaticMeshSourceModel& SrcModel = StaticMesh->AddSourceModel();
    SrcModel.BuildSettings.bRecomputeNormals = false;

    FMeshDescription MeshDescription;
    FStaticMeshAttributes StaticMeshAttributes(MeshDescription);
    StaticMeshAttributes.Register();

    const FDynamicMesh3* Mesh = GetDynamicMesh()->GetMeshPtr();

    FDynamicMeshToMeshDescription Converter;
    Converter.Convert(Mesh, MeshDescription);
    
    // Build the static mesh render data, one FMeshDescription* per LOD.
    StaticMesh->BuildFromMeshDescriptions({&MeshDescription});
    
    return StaticMesh;
}
