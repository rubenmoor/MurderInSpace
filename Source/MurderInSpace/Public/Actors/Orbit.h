#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "Modes/MyState.h"
#include "Orbit.generated.h"

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
};

USTRUCT(BlueprintType)
struct FControllParameters
{
    GENERATED_BODY()
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    double Eccentricity = 0.;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, DisplayName="P = (H * H)/MU")
    double P = 0.;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly )
    double Energy = 0.;
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

UCLASS()
class MURDERINSPACE_API AOrbit final : public AActor
{
	GENERATED_BODY()
	
public:	
	AOrbit();

	virtual void Tick(float DeltaTime) override;

    static FString MakeOrbitLabel(const AActor* Actor) { return Actor->GetName().Append(TEXT("_Orbit")); }

    UFUNCTION(BlueprintCallable)
    bool GetIsInitialized() const { return bIsInitialized; }
    
    // requires call to `Update` afterwards
    UFUNCTION(BlueprintCallable)
    void SetInitialParams(FVector VecV, FPhysics Physics);

    UFUNCTION(BlueprintCallable)
    void SetEnableVisibility(bool NewBVisibility) { bTrajectoryShowSpline = NewBVisibility; }
    
    // update the orbit given
    // * the location of the owner
    // * `VecVelocity`
    // * `Physics`
    UFUNCTION(BlueprintCallable)
    void Update
        ( FVector DeltaVecV
        , FPhysics Physics
        , FInstanceUI InstanceUI
        );

    void Update(FPhysics Physics, FInstanceUI InstanceUI);

    UFUNCTION(BlueprintCallable)
    void UpdateControllParams(FPhysics Physics);

    UFUNCTION(BlueprintCallable)
    void SpawnSplineMesh
        ( FLinearColor Color
        , ESplineMeshParentSelector ParentSelector
        , FInstanceUI InstanceUI
        );
    
    // user interface
    
    UFUNCTION(BlueprintCallable)
    void UpdateSplineMeshScale(double InScaleFactor);

    UFUNCTION(BlueprintPure)
    FVector GetVecRZero() const { return VecRZero; }
    
    UPROPERTY(BlueprintReadWrite)
    bool bIsVisibleAccelerating = false;

    // this is true, when
    //   * an actor receives the mouse over event
    //   * the pawn of the player does ShowMyTrajectory
    UPROPERTY(BlueprintReadWrite)
    bool bIsVisibleMouseover = false;

    UPROPERTY(BlueprintReadWrite)
    bool bIsVisibleShowMyTrajectory = false;

    UPROPERTY(BlueprintReadWrite)
    bool bIsVisibleToggleMyTrajectory = false;

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
    FVector GetVecR() const { return RP_Body->GetActorLocation(); }

    // whenever a spline mesh merely changes visibility:
    UFUNCTION(BlueprintCallable)
    void UpdateVisibility(const FInstanceUI& InstanceUI);

    // object interaction
    
    // replication

    // TODO: rename 'FreezeSplineKey'
    UFUNCTION(BlueprintCallable)
    void FreezeOrbitState()
    {
        RP_OrbitState = { VecVelocity, GetVecR() };
    }

    UFUNCTION(BlueprintCallable)
    void ToggleIsChanging() { bIsChanging = !bIsChanging; }

    UFUNCTION(BlueprintCallable)
    void DestroyTempSplineMeshes();

    /**
     * @brief constant factor to construct tangents for spline points
     */
    static constexpr double SplineToCircle = 1.6568542494923806; // 4. * (sqrt(2) - 1.);
    
protected:

#if WITH_EDITOR
    virtual FString GetDefaultActorLabel() const override { return GetClass()->GetName(); }
#endif
    
    // event handlers
    virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

#if WITH_EDITOR
    virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

	// components

    // TODO
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
    FControllParameters ControllParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
    FVector VecVelocity = FVector::Zero();
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
    double ScalarVelocity = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
    double VelocityVCircle = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler")
    double RKepler;

    // debugging: monitor the closest distance of the body to its orbit, while updating
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler")
    double DistanceToOrbit = 0.;

    UPROPERTY()
    FVector VecRZero = FVector::Zero();
    
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
    void AddPointsToSpline();

    UFUNCTION(BlueprintPure)
    bool GetVisibility(const FInstanceUI& InstanceUI) const;

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

    // TODO delete
    UPROPERTY()
    TArray<FSplinePoint> SplinePoints;

    // TODO delete
    UFUNCTION()
    void OnRep_SplinePoints()
    {
        AddPointsToSpline();
        Spline->UpdateSpline();
    }

    // TODO: delete
    UPROPERTY(VisibleAnywhere)
    bool bClosedLoop;

    UFUNCTION()
    void OnRep_bClosedLoop()
    {
        Spline->SetClosedLoop(bClosedLoop, true);
    }

    // server only
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    bool bIsChanging = false;

    // custom event handlers
	UFUNCTION(BlueprintCallable)
	void HandleBeginMouseOver(AActor* Actor);
	
	UFUNCTION(BlueprintCallable)
	void HandleEndMouseOver(AActor* Actor);
	
	UFUNCTION(BlueprintCallable)
	void HandleClick(AActor* Actor, FKey Button);

    void SetReadyFlags(EOrbitReady ReadyFlags);
    
    EOrbitReady OrbitReady = EOrbitReady::None;
};
