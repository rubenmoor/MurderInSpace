#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "Lib/UStateLib.h"

#include "OrbitComponent.generated.h"

UINTERFACE(meta=(CannotImplementInterfaceInBlueprint))
class UHasOrbit : public UInterface
{
    GENERATED_BODY()
};

class IHasOrbit
{
    GENERATED_BODY()
    
public:
    virtual UOrbitComponent* GetOrbit() = 0;
    virtual USceneComponent* GetMovableRoot() = 0;
    virtual USceneComponent* GetSplineMeshParent() = 0;
protected:
    virtual void Construction(const FTransform& Transform);
};

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

    FOrbitState() {}

    FOrbitState(FVector InVecR, FVector InVecVelocity)
        : VecR(InVecR), VecVelocity(InVecVelocity)
    {}

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector VecR = FVector::Zero();
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector VecVelocity = FVector::Zero();
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
    void SetCircleOrbit(FVector InVecR, FPhysics Physics);

    // you need to call `Update` afterwards
    UFUNCTION(BlueprintCallable)
    void SetOrbitByParams(FVector InVecR, FVector InVecVelocity, FPhysics Physics);

    UFUNCTION(BlueprintCallable)
    void SetEnableVisibility(bool NewBVisibility) { bTrajectoryShowSpline = NewBVisibility; }

    UFUNCTION(BlueprintCallable)
    bool GetHasBeenSet() { return RP_bHasBeenSet; }
    
    // update the orbit given the location of `MovableRoot`, `VecVelocity` and `Physics`
    UFUNCTION(BlueprintCallable)
    void Update(FPhysics Physics, FInstanceUI InstanceUI);

    UFUNCTION(BlueprintCallable)
    void SpawnSplineMesh(FLinearColor Color, USceneComponent* InParent, FInstanceUI InstanceUI);
    
    // user interface
    
    UFUNCTION(BlueprintCallable)
    void UpdateSplineMeshScale(float InScaleFactor);

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
    float GetCircleVelocity(FPhysics Physics) const;

    UFUNCTION(BlueprintCallable)
    FVector GetVecVelocity() { return VecVelocity; }

    UFUNCTION(BlueprintCallable)
    FVector GetVecRKepler(FPhysics Physics) const { return GetVecR() - Physics.VecF1; }

    UFUNCTION(BlueprintCallable)
    FVector GetVecR() const { return GetOwner<IHasOrbit>()->GetMovableRoot()->GetComponentLocation(); }

    // whenever a spline mesh merely changes visibility:
    UFUNCTION(BlueprintCallable)
    void UpdateVisibility(FInstanceUI InstanceUI);

    // object interaction
    
    UFUNCTION(BlueprintCallable)
    void AddVelocity(FVector VecDeltaV, FPhysics Physics, FInstanceUI InstanceUI);

    // replication

    UFUNCTION(BlueprintCallable)
    void FreezeOrbitState() { RP_OrbitState = { GetVecR(), VecVelocity }; }

    UFUNCTION(BlueprintCallable)
    void SetIsChanging(bool InIsChanging) { bIsChanging = InIsChanging; }

    bool bSkipConstruction = false;
    
protected:
    
    // event handlers

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void BeginPlay() override;

#if WITH_EDITOR
    virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
    virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

    virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
    
    // members
    
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

    // UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Kepler")
    // FVector VecR = FVector::Zero();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
    FVector VecVelocity = FVector::Zero();
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
    float Velocity = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
    float VelocityVCircle = 0;
    
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

    // the only way to set the velocity is by providing a value for `VecVelocity`
    // and call `Update` afterwards
    UFUNCTION(BlueprintCallable)
    void SetVelocity(FVector InVecVelocity) { VecVelocity = InVecVelocity; }

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
    bool GetVisibility(FInstanceUI InstanceUI) const;

    // replication

    UPROPERTY(ReplicatedUsing=OnRep_OrbitState, VisibleAnywhere, BlueprintReadWrite)
    FOrbitState RP_OrbitState;

    UFUNCTION()
    void OnRep_OrbitState();

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

    // server only
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    bool bIsChanging = false;
};
