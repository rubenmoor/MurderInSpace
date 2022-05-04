// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class SPACESCENE_ARGHANION_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

	UMyGameInstance();
	/**
	 * @brief the standard gravitational parameter ALPHA [m^3/s^2] = G * M for different bodies
	 */
	static constexpr float ALPHA_Game = 5e2;
	static constexpr float ALPHA_Ceres = 7e10;
	static constexpr float ALPHA_Moon = 4.8e12;
	static constexpr float ALPHA_Earth = 4e14;
	static constexpr float ALPHA_Sun = 1.3e20;

	// for actors, unreal guaranties sanity for values of x and y within [-1048535, 1048535]
	static constexpr float MAX_WORLDRADIUS_UU = 1.048535e6;

	bool bSetUpWorldDimensionsDone = false;
	
protected:
	// event handlers
	
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
public:
	// default values for editor

	// world radius in unreal units
	static constexpr float EditorDefaultWorldRadiusUU = MAX_WORLDRADIUS_UU;
	
	// desired maximum size of the game's world in meters
	static constexpr float EditorDefaultWorldRadiusMeters = 1.048535e4; // 10 km
	
	// 1 m (one real-world meter) = `SCALE_FACTOR` * 1 UE (1 Unreal unit)
	static constexpr float EditorDefaultScaleFactor = EditorDefaultWorldRadiusMeters / EditorDefaultWorldRadiusUU;
	
	// the default value for the standard gravitational parameters
	static constexpr float EditorDefaultAlpha = ALPHA_Game / (EditorDefaultScaleFactor * EditorDefaultScaleFactor * EditorDefaultScaleFactor);

	// the center of mass of the central body
	// TODO: update orbits upon change of VecF1 by game instance/game mode
	inline static const FVector EditorDefaultVecF1 = FVector(0, 0, 0);
	
	//static constexpr float SCALE_FACTOR = WorldRadiusMeters / WorldRadius;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ScaleFactor;
	
	UPROPERTY(EditAnywhere, BlueprintReadwrite)
	float Alpha;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float WorldRadius;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float WorldRadiusMeters;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector VecF1;
	
	void SetUpWorldDimensions(float _WorldRadiusMeters, float _WorldRadius);
	void SetUpCentralBody(float _Alpha, FVector _VecF1);
};
