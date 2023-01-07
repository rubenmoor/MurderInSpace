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
class MURDERINSPACE_API AMyActor_StaticMesh final
	: public AActor
	, public IHasMesh
	, public IHasOrbit
	, public IHasOrbitColor
{
	GENERATED_BODY()

	AMyActor_StaticMesh();

public:
	virtual UPrimitiveComponent* GetMesh()  const override { return StaticMesh; }
	virtual TSubclassOf<AOrbit>  GetOrbitClass()  override { return OrbitClass;   }
	virtual FLinearColor         GetOrbitColor()  override { return OrbitColor;   }
	virtual AOrbit*				 GetOrbit() const override { return RP_Orbit; };
	virtual void 				 SetOrbit(AOrbit* InOrbit) override { RP_Orbit = InOrbit; };
	
protected:
	// event handlers
	virtual void Destroyed() override;
	virtual void OnConstruction(const FTransform& Transform) override;
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

	// components

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> StaticMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<class UGyrationComponent> Gyration;

	// members
	
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category="Orbit")
	AOrbit* RP_Orbit = nullptr;
	
	UPROPERTY(EditDefaultsOnly, Category="Orbit")
	TSubclassOf<AOrbit> OrbitClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Orbit")
    FLinearColor OrbitColor;
};
