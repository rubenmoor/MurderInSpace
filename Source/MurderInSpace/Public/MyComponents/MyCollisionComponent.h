// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MyCollisionComponent.generated.h"

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

	void HandleHit(const FHitResult& HitResult);
protected:
	// event handlers

public:	
};
