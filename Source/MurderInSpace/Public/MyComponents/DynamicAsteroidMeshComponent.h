// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/DynamicMeshComponent.h"
#include "DynamicAsteroidMeshComponent.generated.h"

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UDynamicAsteroidMeshComponent : public UDynamicMeshComponent
{
	GENERATED_BODY()

public:
    UStaticMesh* MakeStaticMesh();

    // TODO: parameters for dynamic mesh generation
};
