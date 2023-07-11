// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MyCollisionComponent.generated.h"

UENUM()
enum class EMyCollisionDimensions : uint8
{
	  CollisionXYPlane
	, CollisionXYZ
};

UINTERFACE(meta=(CannotImplementInterfaceInBlueprint))
class UHasCollision : public UInterface
{
	GENERATED_BODY()
};
class IHasCollision
{
	GENERATED_BODY()
public:
	virtual UMyCollisionComponent* GetCollisionComponent() = 0;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UMyCollisionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UMyCollisionComponent();

	void SetCollisionDimensions(EMyCollisionDimensions InMyCollisionDimensions)
	{
		MyCollisionDimensions = InMyCollisionDimensions;
	}
	
	void HandleHit(FHitResult& HitResult, UPrimitiveComponent* PrimitiveComponent) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double CoR = 1.;

	UFUNCTION(BlueprintCallable)
	double GetMyMass();

	UFUNCTION(BlueprintCallable)
	void UpdateMass(double Radius);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Orbit")
	bool bOverrideMass = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Orbit")
	double MassOverride = 0.;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Orbit")
	double Density = 1.;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Orbit")
	double DensityExponent = 3.;

protected:
	EMyCollisionDimensions MyCollisionDimensions = EMyCollisionDimensions::CollisionXYPlane;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Orbit")
	double MyMass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bMassInitialized;

	// event handlers

#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
};
