// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Orbit.generated.h"

UENUM(BlueprintType)
enum class OrbitType : uint8
{
	CIRCLE UMETA(DisplayName="Circle"),
	ELLIPSE UMETA(DisplayName="Ellipse"),
	LINEBOUND UMETA(DisplayName="LineBound"),
	LINEUNBOUND UMETA(DisplayName="LineUnbound"),
	PARABOLA UMETA(DisplayName="Parabola"),
	HYPERBOLA UMETA(DisplayName="Hyperbola")
};

UCLASS()
class SPACESCENE_ARGHANION_API AOrbit : public AActor
{
	GENERATED_BODY()
	
public:	
	AOrbit();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void UpdateOrbit(FVector VecV, float Alpha, float RMAX);

	void UpdateOrbit(float ALPHA, float RMAX);

	UFUNCTION(BlueprintCallable)
	void SetupActorInSpace(AActor* _ActorInSpace, FVector _VecF1, FVector VecV);

	UFUNCTION(BlueprintCallable)
	FVector GetVecVelocity() { return VecVelocity; };

protected:
	
	// event handlers
	
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedChainEvent) override;
	
	// components
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler")
	TObjectPtr<USplineComponent> Spline;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> HISMTrajectory;

	// members
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	TObjectPtr<AActor> ActorInSpace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	TObjectPtr<UStaticMesh> SM_Trajectory;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler")
	OrbitType Orbit;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler" )
	float Eccentricity;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler", DisplayName="P = (H * H)/MU")
	float P;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler" )
	float Energy;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	FVector VecF1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	float Velocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	FVector VecVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	float VelocityNormalized;

	// period in s
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler")
	float Period;

	// semi-major axis of elliptic orbit
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler")
	float A = 0;

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
	
	/**
	 * @brief constant factor to construct tangents for spline points
	 * //const float CBezierToCircle = 4.0 / 3 * (sqrt(2) - 1);
	 * supposedly the above is the exact formula c = 0.5522
	 * trying out in UE results in 1.65 being optimal
	 */
	static constexpr float SplineToCircle = 1.65;
};
