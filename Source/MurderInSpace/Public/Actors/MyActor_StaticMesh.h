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
	virtual UPrimitiveComponent* GetMesh()       override { return StaticMesh; }
	virtual AOrbit*              GetOrbit()      override { return Orbit;        }
	virtual void       SetOrbit(AOrbit* InOrbit) override { Orbit = InOrbit;     }
	virtual TSubclassOf<AOrbit>  GetOrbitClass() override { return OrbitClass;   }
	virtual FLinearColor         GetOrbitColor() override { return OrbitColor;   }
	
protected:
	// event handlers
	virtual void BeginDestroy() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;

	// components

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> StaticMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<class UGyrationComponent> Gyration;

	// members
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AOrbit> OrbitClass;
	
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
	AOrbit* Orbit;
	
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
    FLinearColor OrbitColor;
};
