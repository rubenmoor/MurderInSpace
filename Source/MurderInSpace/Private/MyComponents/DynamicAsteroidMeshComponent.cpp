// Fill out your copyright notice in the Description page of Project Settings.


#include "MyComponents/DynamicAsteroidMeshComponent.h"
#include "StaticMeshAttributes.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "DynamicMeshToMeshDescription.h"
#include "MeshDescription.h"
#include "Modes/MyState.h"

UDynamicAsteroidMeshComponent::UDynamicAsteroidMeshComponent()
{
    // DistributionNormal = CreateDefaultSubobject<UDistributionFloatConstantCurve>("NormalDistribution");
    // DistributionNormal->bCanBeBaked = true;
    // //DistributionNormal->ConstantCurve = FInterpCurve<float>();
    // DistributionNormal->ConstantCurve.Points = MakePoints<float>
    //     ( { 0. , 0.5, 0.9  , 1.     }
    //     , { 10., 50., 9900., 10000. }
    //     );
}

UStaticMesh* UDynamicAsteroidMeshComponent::MakeStaticMesh(float SizeParam)
{
    GetDynamicMesh()->Reset();
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
        ( GetDynamicMesh()
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

    // TODO: capsule collision
    return StaticMesh;
}

