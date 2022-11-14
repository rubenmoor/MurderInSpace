// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IHasMesh.h"
#include "Orbit.h"
#include "MyActor_StaticMesh.generated.h"

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API AMyActor_StaticMesh final : public AActor, public IHasMesh, public IHasOrbit
{
	GENERATED_BODY()

	AMyActor_StaticMesh();

public:
	virtual UPrimitiveComponent* GetMesh()  override { return StaticMesh; }
	virtual AOrbit*              GetOrbit() override { return Orbit;        }

	UFUNCTION(BlueprintCallable, CallInEditor)
	void CreateOrbit();
	
protected:
	// event handlers
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	// maybe: implement posteditchangedchainproperty to update orbit on change of actor location

	// components

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> StaticMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<class UGyrationComponent> Gyration;

	// members
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	AOrbit* Orbit;
};
