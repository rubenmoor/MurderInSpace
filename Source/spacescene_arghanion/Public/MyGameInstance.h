// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "KeplerOrbitComponent.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

DECLARE_EVENT_OneParam(UMyGameInstance, FOnChangedMu, float)

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

	//FEventChangedMu OnChangedMu() { return EventChangedMu; }
	FOnChangedMu OnChangedMu;
	
protected:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
};
