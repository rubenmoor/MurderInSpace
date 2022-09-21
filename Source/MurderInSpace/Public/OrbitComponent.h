// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "OrbitComponent.generated.h"

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

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UOrbitComponent : public USplineComponent
{
	GENERATED_BODY()

public:
	UOrbitComponent();
	
	UFUNCTION(BlueprintCallable)
	void Update(float Alpha, float WorldRadius, FVector VecF1);

	UFUNCTION(BlueprintCallable)
	void AddVelocity(FVector _VecVelocity, float Alpha, float WorldRadius, FVector VecF1);
	
	UFUNCTION(BlueprintCallable)
	FString GetParamsString();

	UFUNCTION(BlueprintCallable)
	float GetVelocity() { return Velocity; }
	
	UFUNCTION(BlueprintCallable)
	float GetCircleVelocity(float Alpha, FVector VecF1) const;

	UFUNCTION(BlueprintCallable)
	FVector GetVecVelocity() { return VecVelocity; }
	
protected:
	
	// event handlers
	#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
	#endif
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	// members
	
	// pointer to the physical body that orbits and has
	// simulated physics; it can be anywhere in the scene tree of the
	// owning actor
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UPrimitiveComponent> Body;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	bool bTrajectoryShowSpline = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMaterialInstance> SplineMeshMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	float splineMeshLength = 1000.0;
	
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector VecVelocity;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Velocity;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VelocityVCircle;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bInitialized = false;
	
	/**
	 * @brief constant factor to construct tangents for spline points
	 * //const float CBezierToCircle = 4.0 / 3 * (sqrt(2) - 1);
	 * supposedly the above is the exact formula c = 0.5522
	 * trying out in UE results in 1.65 being optimal
	 */
	static constexpr float SplineToCircle = 1.65;

	// private methods
	
	UFUNCTION(BlueprintCallable)
	FVector GetVecR() const { return Body->GetComponentLocation(); };

	UFUNCTION(BlueprintCallable)
	void SetVelocity(FVector _VecVelocity, float Alpha, FVector VecF1);

	UFUNCTION(BlueprintCallable)
	float VelocityEllipse(float R, float Alpha);

	UFUNCTION(BlueprintCallable)
	float VelocityParabola(float R, float Alpha);
	
	UFUNCTION(BlueprintCallable)
	float NextVelocity(float R, float Alpha, float OldVelocity, float DeltaTime, float Sign);

	UFUNCTION(BlueprintCallable)
	FOrbitParameters GetParams() const { return Params; };

};
