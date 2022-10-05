// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "UStateLib.h"

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
	void SetMovableRoot(USceneComponent* InMovableRoot) { MovableRoot = InMovableRoot; }

	UFUNCTION(BlueprintCallable)
	void SetSplineMeshParent(USceneComponent* InSplineMeshParent) { SplineMeshParent = InSplineMeshParent; }

	// whenever there's a change in velocity or the physical constants:
	UFUNCTION(BlueprintCallable)
	void Update(FPhysics Physics, FPlayerUI PlayerUI);

	UFUNCTION(BlueprintCallable)
	void UpdateSplineMeshScale(float InScaleFactor);

	// whenever a splinemesh merely changes visibility:
	UFUNCTION(BlueprintCallable)
	void UpdateVisibility(FPlayerUI PlayerUI);

	UFUNCTION(BlueprintCallable)
	void AddVelocity(FVector VecDeltaV, FPhysics Physics, FPlayerUI PlayerUI);
	
	UFUNCTION(BlueprintCallable)
	FString GetParamsString();

	UFUNCTION(BlueprintCallable)
	float GetVelocity() { return Velocity; }
	
	UFUNCTION(BlueprintCallable)
	float GetCircleVelocity(float Alpha, FVector VecF1) const;

	UFUNCTION(BlueprintCallable)
	FVector GetVecVelocity() { return VecVelocity; }

	UFUNCTION(BlueprintCallable)
	FVector GetVecR() { return VecR; }

	UFUNCTION(BlueprintCallable)
	void InitializeCircle(FVector NewVecR, FPhysics Physics, FPlayerUI PlayerUI);

	UFUNCTION(BlueprintCallable)
	void SpawnSplineMesh(FLinearColor Color, USceneComponent* InParent, FPlayerUI PlayerUI);
	
	UPROPERTY(BlueprintReadWrite)
	bool bIsSelected = false;

	UPROPERTY(BlueprintReadWrite)
	bool bIsVisibleAccelerating = false;

	// this is true, when
	//   * an actor receives the mouse over event
	//   * the pawn of the player does ShowMyTrajectory
	//   * the visibility is changed in the editor
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsVisibleVarious = false;
	
protected:
	
	// event handlers

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void BeginPlay() override;

	#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
	#endif
	
	// members
	
	UPROPERTY(BlueprintReadWrite, Category="Kepler")
	TObjectPtr<USceneComponent> SplineMeshParent;

	UPROPERTY(BlueprintReadWrite, Category="Kepler")
	TObjectPtr<USceneComponent> MovableRoot;

	/* expose a blueprint property to disable orbit visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	bool bTrajectoryShowSpline = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	TObjectPtr<UMaterial> MSplineMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	FLinearColor SplineMeshColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	float SplineMeshScaleFactor = 1.;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	float SplineMeshLength = 1000.0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	TObjectPtr<UStaticMesh> SMSplineMesh;

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
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Kepler")
	float DistanceZero;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Kepler")
	FVector VecR;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	FVector VecVelocity;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	float Velocity;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	float VelocityVCircle;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Kepler")
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
	void SetVelocity(FVector _VecVelocity, float Alpha, FVector VecF1);

	UFUNCTION(BlueprintCallable)
	float VelocityEllipse(float R, float Alpha);

	UFUNCTION(BlueprintCallable)
	float VelocityParabola(float R, float Alpha);
	
	UFUNCTION(BlueprintCallable)
	float NextVelocity(float R, float Alpha, float OldVelocity, float DeltaTime, float Sign);

	UFUNCTION(BlueprintCallable)
	FOrbitParameters GetParams() const { return Params; };
	
	UFUNCTION(BlueprintCallable)
	void MyAddPoints(TArray<FSplinePoint> InSplinePoints, bool bUpdateSpline);

	UFUNCTION(BlueprintPure)
	bool GetVisibility(FPlayerUI PlayerUI) const;
};
