// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyActor.h"
#include "IHasMesh.h"
#include "MyActor_StaticMesh.generated.h"

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API AMyActor_StaticMesh final : public AMyActor, public IHasMesh
{
	GENERATED_BODY()

	AMyActor_StaticMesh();

public:
	virtual UPrimitiveComponent* GetMesh() override final { return StaticMesh; }
	
protected:
	// event handlers
	virtual void BeginPlay() override;

	// components

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> StaticMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<class UGyrationComponent> Gyration;
};
