// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "KeplerOrbitComponent.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class SPACESCENE_ARGHANION_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	/**
	 * @brief the standard gravitational parameter mu = G M
	 *        solar system: 1.3e20
	 *        earth: 4.0e14
	 */
	UPROPERTY(EditAnywhere, BlueprintReadwrite)
	float MU = UKeplerOrbitComponent::DefaultMU;

	static constexpr float DefaultGameAreaRadius = 500000;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GameAreaRadius = DefaultGameAreaRadius;

protected:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
};
