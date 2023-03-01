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
	
	void HandleHit(FHitResult& HitResult);
	
protected:
	EMyCollisionDimensions MyCollisionDimensions = EMyCollisionDimensions::CollisionXYPlane;
	// event handlers

public:	
};
