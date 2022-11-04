// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "Lib/UStateLib.h"

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

/*
 * all the data that makes up the orbit
 * needed for network replication
 */
USTRUCT(BlueprintType)
struct FOrbitState
{
	GENERATED_BODY()

	FOrbitState(): Params(), Velocity(0), SplineKey(0), SplineDistance(0)
	{
	}

	FOrbitState(FName InOrbitName, FVector InVecR, FVector InVecVelocity, FOrbitParameters InParams, float InVelocity, float InSplineKey, float InSplineDistance)
		: OrbitFName(InOrbitName), VecR(InVecR), VecVelocity(InVecVelocity), Params(InParams), Velocity(InVelocity), SplineKey(InSplineKey), SplineDistance(InSplineDistance)
	{
	}

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FName OrbitFName;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector VecR;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector VecVelocity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FOrbitParameters Params;

	// this data can be computed, too, to save network bandwidth, but it's just three floats
	// float Velocity = VecVelocity.Length()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Velocity;
	
	// SplineKey = FindInputKeyClosestToWorldLocation(VecR);
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float SplineKey;
	
	// SplineDistance = GetDistanceAlongSplineAtSplineInputKey(SplineKey);
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float SplineDistance;
};

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UOrbitComponent : public USplineComponent
{
	GENERATED_BODY()

	friend class AMyPlayerController;
public:
	UOrbitComponent();

	// initialization
	
	UFUNCTION(BlueprintCallable)
	void SetMovableRoot(USceneComponent* InMovableRoot) { MovableRoot = InMovableRoot; }

	UFUNCTION(BlueprintCallable)
	void SetSplineMeshParent(USceneComponent* InSplineMeshParent) { SplineMeshParent = InSplineMeshParent; }

	UFUNCTION(BlueprintCallable)
	void SetCircleOrbit(FVector NewVecR, FPhysics Physics, FPlayerUI PlayerUI);

	UFUNCTION(BlueprintCallable)
	void SetOrbitByParams(FVector NewVecR, FVector NewVecVelocity, FPhysics Physics, FPlayerUI PlayerUI);

	UFUNCTION(BlueprintCallable)
	void SetEnableVisibility(bool NewBVisibility) { bTrajectoryShowSpline = NewBVisibility; }

	UFUNCTION(BlueprintCallable)
	bool GetHasBeenSet() { return RP_bHasBeenSet; }
	
	// whenever there's a change in location, velocity, or the physical constants:
	UFUNCTION(BlueprintCallable)
	void Update(FPhysics Physics, FPlayerUI PlayerUI);

	UFUNCTION(BlueprintCallable)
	void SpawnSplineMesh(FLinearColor Color, USceneComponent* InParent, FPlayerUI PlayerUI);
	
	// user interface
	
	UFUNCTION(BlueprintCallable)
	void UpdateSplineMeshScale(float InScaleFactor);

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

	// whenever a splinemesh merely changes visibility:
	UFUNCTION(BlueprintCallable)
	void UpdateVisibility(FPlayerUI PlayerUI);

	// object interaction
	
	UFUNCTION(BlueprintCallable)
	void AddVelocity(FVector VecDeltaV, FPhysics Physics, FPlayerUI PlayerUI);

	// replication

	UFUNCTION(BlueprintCallable)
	void ApplyOrbitState(const FOrbitState& OS);
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

	/* disable orbit visualization entirely */
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

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category="Kepler")
	FOrbitParameters RP_Params;

	// for orbit == LINEBOUND, the spline distance is used because the spline key closest to location cannot be reliably
	// determined, i.e. the object jumps between the two directions
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler")
	float SplineDistance;
	
	// a spline key: the position on the spline; in favor of spline distance which results in inaccuracies
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler")
	float SplineKey;

	// spline distance at last orbit update
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category="Kepler")
	float RP_DistanceZero;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Kepler")
	FVector VecR;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	FVector VecVelocity;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	float Velocity;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	float VelocityVCircle;
	
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category="Kepler")
	bool RP_bHasBeenSet = false;
	
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
	FOrbitParameters GetParams() const { return RP_Params; };
	
	UFUNCTION(BlueprintCallable)
	void MyAddPoints();

	UFUNCTION(BlueprintPure)
	bool GetVisibility(FPlayerUI PlayerUI) const;

	// replication

	UPROPERTY(ReplicatedUsing=OnRep_SplinePoints)
	TArray<FSplinePoint> RP_SplinePoints;

	UFUNCTION()
	void OnRep_SplinePoints()
	{
		MyAddPoints();
		UpdateSpline();
	}

	UPROPERTY(ReplicatedUsing=OnRep_bClosedLoop)
	bool RP_bClosedLoop;

	UFUNCTION()
	void OnRep_bClosedLoop()
	{
		SetClosedLoop(RP_bClosedLoop, true);
	}
};
