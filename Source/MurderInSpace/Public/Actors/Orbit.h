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

UCLASS()
class MURDERINSPACE_API AOrbit final : public AActor
{
	GENERATED_BODY()
	
public:	
	AOrbit();

	virtual void Tick(float DeltaTime) override;

    static FString MakeOrbitLabel(const AActor* Actor) { return Actor->GetName().Append(TEXT("_Orbit")); }
    
    // initialization

    // requires call to `Update` afterwards
    // TODO: make editor callable version with default physics
    UFUNCTION(BlueprintCallable)
    void SetCircleOrbit(FPhysics Physics);

    UFUNCTION(BlueprintCallable)
    void SetEnableVisibility(bool NewBVisibility) { bTrajectoryShowSpline = NewBVisibility; }
    
    // update the orbit given
    // * the location of the owner
    // * `VecVelocity`
    // * `Physics`
    UFUNCTION(BlueprintCallable)
    void Update(FPhysics Physics, FInstanceUI InstanceUI);

    UFUNCTION(BlueprintCallable)
    void SpawnSplineMesh(FLinearColor Color, ESplineMeshParentSelector ParentSelector, FInstanceUI InstanceUI);
    
    // user interface
    
    UFUNCTION(BlueprintCallable)
    void UpdateSplineMeshScale(float InScaleFactor);

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

    //UFUNCTION(BlueprintCallable)
    float GetScalarVelocity() { return ScalarVelocity; }
    
    UFUNCTION(BlueprintCallable)
    float GetCircleVelocity(FPhysics Physics) const;

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
    
    UFUNCTION(BlueprintCallable)
    void AddVelocity(FVector VecDeltaV, FPhysics Physics, FInstanceUI InstanceUI);

    // replication

    UFUNCTION(BlueprintCallable)
    void FreezeOrbitState()
    {
        RP_OrbitState = { GetVecR(), VecVelocity };
    }

    UFUNCTION(BlueprintCallable)
    void ToggleIsChanging() { bIsChanging = !bIsChanging; }

    UFUNCTION(BlueprintCallable)
    void DestroyTempSplineMeshes();

    virtual void PostInitializeComponents() override;
    
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

    virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
    
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler")
    TObjectPtr<UMaterial> SplineMeshMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
    float SplineMeshScaleFactor = 1.;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
    float SplineMeshLength = 1000.0;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Kepler")
    TObjectPtr<UStaticMesh> StaticMesh;

    UPROPERTY(Replicated, BlueprintReadOnly, Category="Kepler")
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
    FVector VecVelocity = FVector::Zero();
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
    float ScalarVelocity = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
    float VelocityVCircle = 0;
    
    /**
     * @brief constant factor to construct tangents for spline points
     * //const float CBezierToCircle = 4.0 / 3 * (sqrt(2) - 1);
     * supposedly the above is the exact formula c = 0.5522
     * trying out in UE results in 1.65 being optimal
     */
    static constexpr float SplineToCircle = 1.65;
    
    // private methods

    // server calls 'Initialize' in 'BeginPlay'
    // clients call 'Initialize' in 'PostNetInit'
    UFUNCTION(BlueprintCallable)
    void Initialize();
    
    UFUNCTION(BlueprintCallable)
    float VelocityEllipse(float R, float Alpha);

    UFUNCTION(BlueprintCallable)
    float VelocityParabola(float R, float Alpha);
    
    UFUNCTION(BlueprintCallable)
    float NextVelocity(float R, float Alpha, float OldVelocity, float DeltaTime, float Sign);

    UFUNCTION(BlueprintCallable)
    FOrbitParameters GetParams() const { return RP_Params; };
    
    // add spline points, in world coordinates, to the spline
    // and to the array `RP_SplinePoints`, for replication
    // however: this one only corrects for the translation, not for rotation and scale
    // as long as we don't scale or rotate the spline, this is fine
    UFUNCTION(BlueprintCallable)
    void AddPointsToSpline();

    UFUNCTION(BlueprintPure)
    bool GetVisibility(const FInstanceUI& InstanceUI) const;

    // replication

    UPROPERTY(ReplicatedUsing=OnRep_OrbitState, VisibleAnywhere, BlueprintReadWrite)
    FOrbitState RP_OrbitState;

    // OrbitState is replicated with condition "initial only", implying that replication (including the call
    // to this method) happens only once
    UFUNCTION()
    void OnRep_OrbitState();

    UFUNCTION()
    void OnRep_Body();

    UPROPERTY(ReplicatedUsing=OnRep_SplinePoints)
    TArray<FSplinePoint> RP_SplinePoints;

    UFUNCTION()
    void OnRep_SplinePoints()
    {
        AddPointsToSpline();
        Spline->UpdateSpline();
    }

    UPROPERTY(ReplicatedUsing=OnRep_bClosedLoop)
    bool RP_bClosedLoop;

    UFUNCTION()
    void OnRep_bClosedLoop()
    {
        Spline->SetClosedLoop(RP_bClosedLoop, true);
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
    
    EOrbitReady OrbitReady;
};
