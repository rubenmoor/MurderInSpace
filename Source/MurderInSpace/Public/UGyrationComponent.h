// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UGyrationComponent.generated.h"

/*
 * physical rotation of objects in space
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MURDERINSPACE_API UUGyrationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUGyrationComponent();

protected:
	// private methods
	
	
	// event handlers
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
