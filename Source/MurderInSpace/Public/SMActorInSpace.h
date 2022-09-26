// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActorInSpace.h"
#include "SMActorInSpace.generated.h"

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API ASMActorInSpace : public AActorInSpace
{
	GENERATED_BODY()

	ASMActorInSpace();
	
protected:
	// event handlers

	// components

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> StaticMesh;
};
