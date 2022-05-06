// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "Orbit.generated.h"

class UOrbitDataComponent;

UENUM(BlueprintType)
enum class EOrbitType : uint8
{
	CIRCLE UMETA(DisplayName="Circle"),
	ELLIPSE UMETA(DisplayName="Ellipse"),
	LINEBOUND UMETA(DisplayName="LineBound"),
	LINEUNBOUND UMETA(DisplayName="LineUnbound"),
	PARABOLA UMETA(DisplayName="Parabola"),
	HYPERBOLA UMETA(DisplayName="Hyperbola")
};

USTRUCT(BlueprintType)
struct FOrbitParameters
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EOrbitType OrbitType;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly )
	float Eccentricity;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, DisplayName="P = (H * H)/MU")
	float P;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly )
	float Energy;
	
	// period in s
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Period;

	// semi-major axis of elliptic orbit
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float A = 0;
};

USTRUCT(BlueprintType)
struct FNewVelocityAndLocation
{
	GENERATED_BODY()
	FVector NewVecVelocity;
	FVector NewLocation;
};

UCLASS()
class SPACESCENE_ARGHANION_API AOrbit : public AActor
{
	GENERATED_BODY()
	
public:	
	AOrbit();

	UFUNCTION(BlueprintCallable)
	void SetOrbitData(UOrbitDataComponent* _OrbitData) { OrbitData = _OrbitData; };

	UFUNCTION(BlueprintCallable)
	void Update(float Alpha, float WorldRadius, FVector VecF1);

	UFUNCTION(BlueprintCallable)
	float VelocityEllipse(float R, float Alpha);

	UFUNCTION(BlueprintCallable)
	float VelocityParabola(float R, float Alpha);
	
	UFUNCTION(BlueprintCallable)
	float NextVelocity(float R, float Alpha, float OldVelocity, float DeltaTime, float Sign);

	UFUNCTION(BlueprintCallable)
	FNewVelocityAndLocation AdvanceOnSpline(float DeltaR, float Velocity, FVector VecR, float DeltaTime);

	UFUNCTION(BlueprintCallable)
	FOrbitParameters GetParams() const { return Params; };

	UFUNCTION(BlueprintCallable)
	FString GetParamsString();
	
protected:
	
	// event handlers
	
	// components
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler")
	TObjectPtr<USplineComponent> Spline;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> HISMTrajectory;

	// members
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	TObjectPtr<UOrbitDataComponent> OrbitData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	TObjectPtr<UStaticMesh> SM_Trajectory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler")
	FOrbitParameters Params;

	// for orbit == LINEBOUND, the spline distance is used because the spline key closest to location cannot be reliably
	// determined, i.e. the object jumps between the two directions
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler")
	float SplineDistance;
	
	// a spline key: the position on the spline; in favor of spline distance which results in inaccuracies
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler")
	float SplineKey;

	// distance travelled at orbit creation
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float DistanceZero;

	// HISM marker index; counts higher than the number of instances
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 HISMCurrentIndex;

	// HISM distance between markers
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float HISMDistance = 40;

	// HISM Number of markers
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int HISMNumberOfMarkers;

	// HISM maximum length
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float HISMMaxLength = 2500;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bInitialized = false;
	
	/**
	 * @brief constant factor to construct tangents for spline points
	 * //const float CBezierToCircle = 4.0 / 3 * (sqrt(2) - 1);
	 * supposedly the above is the exact formula c = 0.5522
	 * trying out in UE results in 1.65 being optimal
	 */
	static constexpr float SplineToCircle = 1.65;
};
