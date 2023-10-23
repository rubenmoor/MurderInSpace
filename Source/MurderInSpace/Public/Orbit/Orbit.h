#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "Logging/StructuredLog.h"
#include "Modes/MyGameInstance.h"
#include "Modes/MyGameState.h"

#include "Orbit.generated.h"

struct FInitialOrbitParams;

UENUM()
enum class EMotionEquation : uint8
{
      FollowSpline
    , NewtonianLocation
    , NewtonianVelocity
};

UENUM(meta=(Bitflags))
enum class EOrbitReady : uint8
{
    None           = 0,
    InternalReady  = 1 << 0,
    BodyReady      = 1 << 1,

    All = InternalReady | BodyReady
};
ENUM_CLASS_FLAGS(EOrbitReady)

UENUM()
enum class ESplineMeshParentSelector : uint8
{
    Temporary,
    Permanent
};

UINTERFACE(meta=(CannotImplementInterfaceInBlueprint))
class UHasOrbit : public UInterface
{
    GENERATED_BODY()
};

class IHasOrbit
{
    GENERATED_BODY()
    
public:
    virtual TSubclassOf<AOrbit> GetOrbitClass() = 0;
    virtual void OrbitSetup(AActor* Actor);
    virtual AOrbit* GetOrbit() const = 0;
    virtual void SetOrbit(AOrbit* Orbit) = 0;
    virtual FInitialOrbitParams GetInitialOrbitParams() const = 0;
    virtual void SetInitialOrbitParams(const FInitialOrbitParams& InParams) = 0;
};

UINTERFACE(meta=(CannotImplementInterfaceInBlueprint))
class UHasOrbitColor : public UInterface
{
    GENERATED_BODY()
};

class IHasOrbitColor
{
    GENERATED_BODY()
    
public:
    virtual FLinearColor GetOrbitColor() = 0;
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
    EOrbitType OrbitType = EOrbitType::CIRCLE;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly )
    FVector VecE = FVector::Zero();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    double Eccentricity = 0.;

    // specific angular momentum
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, DisplayName="H = r x v")
    FVector VecH = FVector::Zero();
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, DisplayName="P = (H * H)/MU")
    double P = 0.;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly )
    double Energy = 0.;
    
    // period in s
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    double Period = 0.;

    // semi-major axis of elliptic orbit
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    double A = 0.;

    void LogMyGameOutput(FString StrContext)
    {
        UE_LOGFMT
            ( LogMyGame
            , Display
            , "{CTX}: OrbitType: {ORBITTYPE}, VecE: {VECE}, Eccentricity: {ECCENTRICITY}, VecH: {VECH}, P: {P}, Energy: {ENERGY}, Period: {PERIOD}, A: {A}"
            , StrContext
            , UEnum::GetValueAsString(OrbitType), VecE.ToString(), Eccentricity, VecH.ToString(), P, Energy, Period, A
            );
    }
};

USTRUCT(BlueprintType)
struct FControlParameters
{
    GENERATED_BODY()
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    double Eccentricity = 0.;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, DisplayName="P = (H * H)/MU")
    double P = 0.;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    double Energy = 0.;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    double DistanceToSpline = 0.;
};

/*
 * all the data that makes up the orbit
 * needed for network replication
 */
USTRUCT(BlueprintType)
struct FOrbitState
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    FVector VecR = FVector::Zero();
    
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    FVector VecVelocity = FVector::Zero();
};

/*
 * The initial params to define the starting orbit
 * Any actor that implements 'IHasOrbit' needs to provide this
 */
USTRUCT(BlueprintType)
struct FInitialOrbitParams
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FVector VecEccentricity = FVector::Zero();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FVector VecHNorm = FVector(0., 0., 1.);

    // the velocity is only relevant when H = 0
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FVector VecVelocity = FVector(0., 0., 0.);
};

UCLASS()
class MURDERINSPACE_API AOrbit final : public AActor
{
	GENERATED_BODY()
	
public:	
	AOrbit();

	virtual void Tick(float DeltaTime) override;

    // set offset vector
    // the orbit doesn't need an update a new actor location is compensated by a new offset vector
    void AddOffset(const FVector VecDeltaOffset) { VecOffset += VecDeltaOffset; }
    
    UFUNCTION(BlueprintCallable)
    void SetDrawDebug(bool bDraw) { Spline->SetDrawDebug(bDraw); }
    
    static FString MakeOrbitLabel(const AActor* Actor) { return Actor->GetName().Append(TEXT("_Orbit")); }

    UFUNCTION(BlueprintCallable)
    bool GetIsInitialized() const { return bIsInitialized; }

    UFUNCTION(BlueprintCallable)
    void UpdateByInitialParams(FPhysics Physics);

    // requires call to `Update` afterwards
    UFUNCTION(BlueprintCallable)
    void SetVelocity(FVector VecV) { VecVelocity = VecV; }

    UFUNCTION(BlueprintCallable)
    void SetEnableVisibility(bool NewBVisibility) { bTrajectoryShowSpline = NewBVisibility; }
    
    /* update the orbit given
     *     `DeltaVecV`: a change in velocity
     *     `Physics`
     */
    UFUNCTION(BlueprintCallable)
    void Update
        ( FVector DeltaVecV
        , FPhysics Physics
        );

    /*
     * update the orbit given
     * `DeltaVecV`: a change in velocity
     * `InVecOffset`: point from the physically correct center-of-mass to the actor location
     * `InOmegaOffset`: [1/s], angular velocity to have the offset vector rotate
     */
    void Update
        ( FVector DeltaVecV
        , FVector InVecOffset
        , double InOmegaOffset
        , FPhysics Physics
        );

    void Update(FPhysics Physics);

    UFUNCTION(BlueprintCallable)
    void UpdateControlParams(FPhysics Physics);

    UFUNCTION(BlueprintCallable)
    void SpawnSplineMesh
        ( FLinearColor Color
        , ESplineMeshParentSelector ParentSelector
        );
    
    // user interface
    
    UFUNCTION(BlueprintCallable)
    void UpdateSplineMeshScale(double InScaleFactor);
    
    UPROPERTY(BlueprintReadWrite)
    int32 VisibilityCount = 0;
    
#if WITH_EDITORONLY_DATA
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsVisibleInEditor = false;
#endif
    
    UFUNCTION(BlueprintCallable)
    FString GetParamsString();

    UFUNCTION(BlueprintPure)
    double GetScalarVelocity() const { return ScalarVelocity; }
    
    UFUNCTION(BlueprintCallable)
    FVector GetCircleVelocity(FPhysics Physics);

    UFUNCTION(BlueprintCallable)
    FVector GetVecVelocity() { return VecVelocity; }

    UFUNCTION(BlueprintCallable)
    FVector GetVecRKepler(FPhysics Physics) const { return GetVecR() - Physics.VecF1; }

    UFUNCTION(BlueprintCallable)
    FVector GetVecR() const;

    UFUNCTION(BlueprintCallable)
    void UpdateVisibility(bool InShow);

    // object interaction
    
    // replication

    UFUNCTION(BlueprintCallable)
    void FreezeOrbitState()
    {
        RP_OrbitState = { GetVecR(), VecVelocity};
    }

    UFUNCTION(BlueprintCallable)
    void DestroyTempSplineMeshes();

    UFUNCTION(BlueprintCallable)
    USplineComponent* GetSplineComponent() { return Spline; }

    UFUNCTION(BlueprintCallable)
    void SetEnabled(bool InEnabled);

protected:

#if WITH_EDITOR
    virtual FString GetDefaultActorLabel() const override { return GetClass()->GetName(); }
#endif
    
    // event handlers
    virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

    virtual void BeginDestroy() override;

#if WITH_EDITOR
    virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

	// components
public:
    UPROPERTY(ReplicatedUsing=OnRep_Body, VisibleAnywhere, BlueprintReadOnly)
    AActor* RP_Body = nullptr;
protected:
	
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USceneComponent> Root;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USplineComponent> Spline;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USceneComponent> SplineMeshParent;

    // only used by possessed pawns
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USceneComponent> TemporarySplineMeshParent;

    // members
    
    /* disable orbit visualization entirely */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
    bool bTrajectoryShowSpline = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Kepler")
    TObjectPtr<UMaterial> SplineMeshMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
    double SplineMeshScaleFactor = 1.;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
    double SplineMeshLength = 100000.0;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Kepler")
    TObjectPtr<UStaticMesh> StaticMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler")
    FOrbitParameters Params;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler")
    FControlParameters ControlParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
    FVector VecVelocity = FVector::Zero();
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
    double ScalarVelocity = 0.;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
    double VelocityVCircle = 0.;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler")
    double RKepler;

    // point from physically correct center-of-mass to actor location
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Kepler")
    FVector VecOffset = FVector::ZeroVector;

    // [1/s], have the offset vector rotate
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Kepler")
    double OmegaOffset = 0.;

    // the bigger this value, the earlier an eccentricity approaching 1 will be interpreted as parabola orbit
    // which results in smoother orbits
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
    double EccentricityTolerance = 1e-8;

    // private methods

    // server calls 'Initialize' in 'BeginPlay'
    // clients call 'Initialize' when the Orbit is ready,
    // i.e. when both, 'BeginPlay' and 'OnRep_Body' have happened
    UFUNCTION(BlueprintCallable)
    void Initialize();

    UPROPERTY(VisibleAnywhere)
    bool bIsInitialized = false;

    // disable to stop ticking and orbit visualisation
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnabled = true;

    // TODO: deprecate
    // error correction
    // the resulting orbit tends to be off by up to 300 UU, i.e. the body location and the closest point on its orbit
    // ideally, I could correct this by increasing the accuracy of the orbit calculation
    // until then, I just move the entire orbit to bring that number down to zero
    UFUNCTION(BlueprintCallable)
    void CorrectSplineLocation();
    
    UFUNCTION(BlueprintCallable)
    FOrbitParameters GetParams() const { return Params; };
    
    // add spline points, in world coordinates, to the spline
    // and to the array `SplinePoints`, for replication
    // however: this one only corrects for the translation, not for rotation and scale
    // as long as we don't scale or rotate the spline, this is fine
    UFUNCTION(BlueprintCallable)
    void AddPointsToSpline(TArray<FSplinePoint> SplinePoints);

    // replication

    // TODO: rename FrozenSplineKey
    UPROPERTY(ReplicatedUsing=OnRep_OrbitState, VisibleAnywhere, BlueprintReadWrite)
    FOrbitState RP_OrbitState;

    // OrbitState is replicated with condition "initial only", implying that replication (including the call
    // to this method) happens only once
    UFUNCTION()
    void OnRep_OrbitState();

    UFUNCTION()
    void OnRep_Body();

    // custom event handlers
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
    float SplineDistance;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Kepler")
    float DistanceToSplineAtUpdate;

    void SetReadyFlags(EOrbitReady ReadyFlags);
    
    EOrbitReady OrbitReady = EOrbitReady::None;

    // minimal velocity mechanism
    // when we move slowly, spline distance is too inaccurate and the body effectively stops
    // thus we define a minimum velocity to always move
    // the minimal velocity: but in terms of spline distance displaced per frame
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
    float MinimalDisplacement = 1.;

    FDelegateHandle ShowAllOrbitsHandle;
};
