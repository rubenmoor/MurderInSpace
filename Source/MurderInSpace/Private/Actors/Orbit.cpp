#include "Actors/Orbit.h"

#include <numeric>

#include "Actors/MyCharacter.h"
#include "Components/SplineMeshComponent.h"
#include "HUD/MyHUD.h"
#include "Lib/FunctionLib.h"
#include "Logging/LogMacros.h"
#include "Modes/MyPlayerController.h"
#include "Modes/MyGameState.h"
#include "Net/UnrealNetwork.h"

void IHasOrbit::OrbitSetup(AActor* Actor)
{
    UMyState* MyState = GEngine->GetEngineSubsystem<UMyState>();

#if WITH_EDITOR
    Actor->SetActorLabel(Actor->GetName());
    
    if(IsValid(GetOrbit()) && GetOrbit()->RP_Body != Actor)
    {
        // this actor has been copied in the editor, to the effect that a new orbit is spawned in a first step
        // but in a second step, the Orbit is overwritten when the old values are copied over
        FString OrbitLabel = AOrbit::MakeOrbitLabel(Actor);
        SetOrbit(*TMyObjectIterator<AOrbit>([Actor, OrbitLabel] (const AOrbit* Orbit)
        {
            return Actor->GetWorld() == Orbit->GetWorld()
                && OrbitLabel == Orbit->GetActorLabel();
        }));
    }
#endif
    
    const FPhysics Physics = MyState->GetPhysicsAny(Actor);
    const FInstanceUI InstanceUI = MyState->GetInstanceUIAny(Actor);

    if(IsValid(GetOrbit()))
    {
        if(GetOrbit()->GetVecR() != GetOrbit()->GetVecRZero())
        {
            // this only ever gets executed when creating objects in the editor (and dragging them around)
            GetOrbit()->SetInitialParams(GetOrbit()->GetCircleVelocity(Physics), Physics);
            GetOrbit()->SetEnableVisibility(Actor->Implements<UHasOrbitColor>());
            GetOrbit()->Update(FVector::Zero(), Physics, InstanceUI);
        }
    }
    else
    {
        if(!IsValid(GetOrbitClass()))
        {
            UE_LOG(LogMyGame, Error, TEXT("%s: OnConstruction: OrbitClass null"), *Actor->GetFullName())
            return;
        }

        UE_LOG(LogMyGame, Warning, TEXT("%s: going to spawn orbit"), *Actor->GetFullName())
        FActorSpawnParameters Params;
        Params.CustomPreSpawnInitalization = [Actor] (AActor* ActorOrbit)
        {
            Cast<AOrbit>(ActorOrbit)->RP_Body = Actor;
        };
        AOrbit* NewOrbit = Actor->GetWorld()->SpawnActor<AOrbit>(GetOrbitClass(), Params);
        SetOrbit(NewOrbit);
        UE_LOG(LogMyGame, Warning, TEXT("%s: orbit spawned: %s"), *Actor->GetFullName(), *NewOrbit->GetFullName())
    }
}

AOrbit::AOrbit()
{
    // only start to tick after PostNetInit
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
    
    bNetLoadOnClient = false;
    bReplicates = true;
    bAlwaysRelevant = true;
    AActor::SetReplicateMovement(false);
    
    Root = CreateDefaultSubobject<USceneComponent>("Root");
    Root->SetMobility(EComponentMobility::Stationary);
    SetRootComponent(Root);
    
    Spline = CreateDefaultSubobject<USplineComponent>("Orbit");
    Spline->SetMobility(EComponentMobility::Stationary);
    Spline->SetupAttachment(Root);

    SplineMeshParent = CreateDefaultSubobject<USceneComponent>("SplineMeshes");
    SplineMeshParent->SetupAttachment(Root);
    SplineMeshParent->SetMobility(EComponentMobility::Stationary);

    TemporarySplineMeshParent = CreateDefaultSubobject<USceneComponent>("TemporarySplineMeshes");
    TemporarySplineMeshParent->SetupAttachment(Root);
    TemporarySplineMeshParent->SetMobility(EComponentMobility::Stationary);
}

void AOrbit::DestroyTempSplineMeshes()
{
    TArray<USceneComponent*> Meshes;
    TemporarySplineMeshParent->GetChildrenComponents(false, Meshes);
    for(USceneComponent* Mesh : Meshes)
    {
        Mesh->DestroyComponent();
    }
}

void AOrbit::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
#if WITH_EDITOR
    if(IsValid(RP_Body))
    {
        SetActorLabel(MakeOrbitLabel(RP_Body), false);
    }
#endif
}

void AOrbit::Initialize()
{
    UMyState* MyState = GEngine->GetEngineSubsystem<UMyState>();

    if(!IsValid(RP_Body))
    {
        UE_LOG(LogMyGame, Error, TEXT("%s: body invalid"), *GetFullName())
        return;
    }
    
    if  (  RP_Body->Implements<UHasMesh>()
        && !(Cast<AMyCharacter>(RP_Body) && Cast<AMyCharacter>(RP_Body)->IsLocallyControlled())
        )
    {
        RP_Body->OnBeginCursorOver.AddDynamic(this, &AOrbit::HandleBeginMouseOver);
        RP_Body->OnEndCursorOver.AddDynamic(this, &AOrbit::HandleEndMouseOver);
        RP_Body->OnClicked.AddDynamic(this, &AOrbit::HandleClick);
    }
    
    MyState->WithInstanceUI(this, [this, MyState] (auto& InstanceUI)
    {
        MyState->WithPhysics(this, [this, &InstanceUI] (auto& Physics)
        {
            // make sure the orbit has the game physics, instead of the editor default physics
            Update(FVector::Zero(), Physics, InstanceUI);
        });
    });
    if(RP_Body->GetLocalRole() == ROLE_AutonomousProxy)
    {
        Cast<AMyCharacter>(RP_Body)->GetController<AMyPlayerController>()->GetHUD<AMyHUD>()->SetReadyFlags(EHUDReady::OrbitReady);
    }

    bIsInitialized = true;
    SetActorTickEnabled(true);
}

void AOrbit::BeginPlay()
{
    Super::BeginPlay();

    UMyState* MyState = GEngine->GetEngineSubsystem<UMyState>();
    
    bool bHasProblems = false;

    // Only care for spline static mesh and material if this orbit is meant to be visible
    if(bTrajectoryShowSpline)
    {
        if(!SplineMeshMaterial)
        {
            UE_LOG(LogMyGame, Warning, TEXT("%s: spline mesh material not set"), *GetFullName())
            bHasProblems = true;
        }
        if(!StaticMesh)
        {
            UE_LOG(LogMyGame, Warning, TEXT("%s: static mesh for trajectory not set"), *GetFullName())
            bHasProblems = true;
        }
    }
    if(bHasProblems)
    {
        return;
    }

    // ignore any of the visibility booleans set in the editor
    bIsVisibleMouseover = false;
    bIsVisibleShowMyTrajectory = false;
    bIsVisibleToggleMyTrajectory = false;

    MyState->WithInstanceUI(this, [this] (FInstanceUI& InstanceUI)
    {
        UpdateVisibility(InstanceUI);
    });

    if(GetLocalRole() == ROLE_Authority)
    {
        Initialize();
    }
    else
    {
        SetReadyFlags(EOrbitReady::InternalReady);
    }

}

void AOrbit::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    //const FPhysics Physics = GEngine->GetEngineSubsystem<UMyState>()->GetPhysicsAny(this);
    AMyGameState* GS = GetWorld()->GetGameState<AMyGameState>();
    const FPhysics Physics = GEngine->GetEngineSubsystem<UMyState>()->GetPhysics(GS);
    const FVector VecRKepler = GetVecRKepler(Physics);

    //ScalarVelocity = NextVelocity(VecRKepler.Length(), Physics.Alpha, ScalarVelocity, DeltaTime, VecVelocity.Dot(VecRKepler));
    //VelocityVCircle = ScalarVelocity / GetCircleVelocity(Physics);

    // calculate velocity and get new location, ...
    ScalarVelocity = sqrt(UFunctionLib::ScalarVelocitySquared(VecRKepler.Length(), RP_Params.A, Physics.Alpha));
    VelocityVCircle = ScalarVelocity / GetCircleVelocity(Physics).Length();
    VecVelocity =
          Spline->GetTangentAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World).GetSafeNormal()
        * ScalarVelocity;
    const float DeltaR = ScalarVelocity * DeltaTime;
    SplineDistance = fmod(SplineDistance + DeltaR, Spline->GetSplineLength());
    const FVector NewVecRKepler = Spline->GetLocationAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World) - Physics.VecF1;
    RKepler = NewVecRKepler.Length();
    
    RP_Body->SetActorLocation(NewVecRKepler + Physics.VecF1);
    
    UpdateControllParams(Physics);
}

void AOrbit::Update(FVector DeltaVecV, FPhysics Physics, FInstanceUI InstanceUI)
{
    const FVector VecR = GetVecR();
    
    // transform location vector r to Kepler coordinates, where F1 is the origin
    const FVector VecRKepler = GetVecRKepler(Physics);

    // the bigger this value, the earlier an eccentricity approaching 1 will be interpreted as parabola orbit
    // which results in smoother orbits
    constexpr float Tolerance = 1E-2;
    
    // the velocity needs recalculation to accurately reflect the current angular momentum
    const FVector VecVNew =
          UFunctionLib::VecVelocity(RP_Params.VecE, VecRKepler, RP_Params.VecH, Physics.Alpha, VecVelocity)
        + DeltaVecV;
    RP_Params.VecH = VecRKepler.Cross(VecVNew);
    RP_Params.VecE = UFunctionLib::Eccentricity(VecRKepler, VecVNew, Physics.Alpha);
    RP_Params.P = RP_Params.VecH.SquaredLength() / Physics.Alpha;

    // not necessary, as the velocity variables are set by the Tick
    // but if not set here, the editor doesn't show the current velocity
    VecVelocity = VecVNew;
    ScalarVelocity = VecVelocity.Length();
    VelocityVCircle = ScalarVelocity / GetCircleVelocity(Physics).Length();
    RKepler = VecRKepler.Length();
    
    RP_Params.Energy = pow(ScalarVelocity, 2) / 2. - Physics.Alpha / RKepler;
    
    float Eccentricity = RP_Params.VecE.Length();
    const FVector VecENorm = RP_Params.VecE.GetSafeNormal();
    const FVector VecHNorm = RP_Params.VecH.GetSafeNormal();

    // the energy of the weakest bound state: a circular orbit at R_MAX
    const float E_BOUND_MIN = -Physics.Alpha / (2 * Physics.WorldRadius);

    // H = 0 (implies E = 1, too): falling in a straight line
    if(VecHNorm.IsZero())
    {
        const float EMIN = -Physics.Alpha / Physics.WorldRadius;
        const float E = VecVelocity.SquaredLength() / 2. - Physics.Alpha / RKepler;

        // bound
        if(E < EMIN)
        {
            const float Apsis = -Physics.Alpha / E;
            const FVector VecVNorm = VecVelocity.GetSafeNormal();
            if(VecVNorm.IsZero())
            {
                RP_SplinePoints =
                    { FSplinePoint(0, VecR, ESplinePointType::Linear)
                    , FSplinePoint(1, -VecRKepler + Physics.VecF1, ESplinePointType::Linear)
                    };
                AddPointsToSpline();
            }
            else
            {
                RP_SplinePoints =
                    { FSplinePoint(0, -VecVNorm * Apsis + Physics.VecF1, ESplinePointType::Linear)
                    , FSplinePoint(1,  VecVNorm * Apsis + Physics.VecF1, ESplinePointType::Linear)
                    };
                AddPointsToSpline();
                Spline->SetClosedLoop(false, false);
                Spline->UpdateSpline();
            }
            
            // the 'SplineKey' is used for both line-type orbits
            SplineDistance = Spline->GetDistanceAlongSplineAtSplineInputKey(Spline->FindInputKeyClosestToWorldLocation(VecR));
            Spline->SetClosedLoop(true, false);
            
            RP_Params.OrbitType = EOrbitType::LINEBOUND;
            RP_Params.A = UFunctionLib::SemiMajorAxis(VecRKepler, VecVelocity, Physics.Alpha);
            RP_Params.Period = UFunctionLib::PeriodEllipse(RP_Params.A, Physics.Alpha);
        }

        // unbound
        else
        {
            RP_SplinePoints =
                { FSplinePoint
                    (0
                    , VecR
                    , ESplinePointType::Linear
                    )
                , FSplinePoint
                    ( 1
                    , VecVelocity.GetUnsafeNormal() *
                        ( Physics.WorldRadius -
                            (VecR).Length()
                        )
                    , ESplinePointType::Linear
                    )
                };
            AddPointsToSpline();

            Spline->SetClosedLoop(false, false);
            RP_Params.OrbitType = EOrbitType::LINEUNBOUND;
            RP_Params.A = -Physics.Alpha / 2 / RP_Params.Energy;
            RP_Params.Period = 0;
        }
    }
    
    // E = 0, circle
    else if(VecENorm.IsZero())
    {
        const FVector VecP2 = VecHNorm.Cross(VecRKepler);
        const FVector VecT1 = VecHNorm.Cross(VecRKepler) * SplineToCircle;
        const FVector VecT4 = VecRKepler * SplineToCircle;
        
        RP_SplinePoints =
            { FSplinePoint(0,  VecR,  VecT1,  VecT1)
            , FSplinePoint(1, VecP2 + Physics.VecF1, -VecT4, -VecT4)
            , FSplinePoint(2, -VecRKepler + Physics.VecF1 , -VecT1, -VecT1)
            , FSplinePoint(3, -VecP2 + Physics.VecF1,  VecT4,  VecT4)
            };
        AddPointsToSpline();
        Spline->SetClosedLoop(true, false);
        RP_Params.OrbitType = EOrbitType::CIRCLE;
        RP_Params.A = RKepler;
        RP_Params.Period = UFunctionLib::PeriodEllipse(RKepler, Physics.Alpha);
    }

    // 0 < E < 1, Ellipse
    // the Tolerance causes a peculiarity:
    // E close to 1 can imply a parabola, but only for Energy approaching 0 (and P > 0);
    // E close to 1 within the given `Tolerance`, allows for Energy * P / ALPHA in the range
    // [ Tolerance^2 / 2 - Tolerance, Tolerance^2 + Tolerance], which is a huge range for Energy, given P,
    // and any bound orbit that should be just a narrow ellipse, jumps to being a wrong parabola;
    // those "narrow ellipses" imply small P values; P / (1 + Eccentricity) equals the periapsis; thus:
    // small values for the periapsis that are impossible Kepler orbits because of collision with the main body.
    // For a black hole, a P cutoff isn't ideal (small periapses are allowed), thus instead I define a cutoff energy:
    // The total energy at R_MAX when orbiting in a circle is the bound state with energy closest to zero.
    // As long the energy is smaller than that, we can safely assume a bound state and thus an ellipse instead of
    // the parabola
    else if(Eccentricity <= 1. - Tolerance || (Eccentricity <= 1 && RP_Params.Energy < E_BOUND_MIN))
    {
        RP_Params.A = RP_Params.P / (1. - pow(Eccentricity, 2));
        const float B = RP_Params.A * sqrt(1 - pow(Eccentricity, 2));
        const FVector Vertex1 = RP_Params.A * (1 - Eccentricity) * VecENorm;
        const FVector Vertex2 = RP_Params.A * (1 + Eccentricity) * -VecENorm;
        const FVector Orthogonal = RP_Params.VecH.Cross(RP_Params.VecE).GetSafeNormal();
        const FVector Covertex1 =  B * Orthogonal - RP_Params.A * RP_Params.VecE;
        const FVector Covertex2 = -B * Orthogonal - RP_Params.A * RP_Params.VecE;
        const FVector T1 = Orthogonal * SplineToCircle * B;
        const FVector T4 = VecENorm * SplineToCircle * RP_Params.A;
        RP_SplinePoints =
            { FSplinePoint(0, Vertex1   + Physics.VecF1,  T1,  T1)
            , FSplinePoint(1, Covertex1 + Physics.VecF1, -T4, -T4)
            , FSplinePoint(2, Vertex2   + Physics.VecF1, -T1, -T1)
            , FSplinePoint(3, Covertex2 + Physics.VecF1,  T4,  T4)
            };
        AddPointsToSpline();
        Spline->SetClosedLoop(true, false);
        RP_Params.OrbitType = EOrbitType::ELLIPSE;
        RP_Params.Period = UFunctionLib::PeriodEllipse(RP_Params.A, Physics.Alpha);
    }
    
    // E = 1, Parabola
    // small tolerance, nothing wrong with hyperbolas
    else if(Eccentricity <= 1.001)
    {
        std::list<FVector> Points;
        const FVector VecHorizontal = VecHNorm.Cross(VecENorm);
        constexpr int MAX_POINTS = 20;
        const float Delta = sqrt(2 * Physics.WorldRadius / RP_Params.P) / ((MAX_POINTS - 1) / 2);

        Points.emplace_front(VecENorm * RP_Params.P / (1. + Eccentricity) + Physics.VecF1);
        for(int i = 1; i < MAX_POINTS / 2; i++)
        {
            const FVector VecX = i * Delta * VecHorizontal * RP_Params.P;
            const FVector VecY = VecENorm / 2. * (1 - pow(i * Delta, 2)) * RP_Params.P;
            Points.emplace_back(VecY + VecX + Physics.VecF1);
            Points.emplace_front(VecY - VecX + Physics.VecF1);
        }

        RP_SplinePoints.Empty();
        RP_SplinePoints.Reserve(Points.size());
        const int32 NumPoints = Points.size();
        for(int i = 0; i < NumPoints; i++)
        {
            RP_SplinePoints.Emplace(i, Points.front());
            Points.pop_front();
        }
        AddPointsToSpline();

        // ClearSplinePoints();
        // for(const FVector Point : Points)
        // {
        // 	AddSplineWorldPoint(Point);
        // }
        
        Spline->SetClosedLoop(false, false);
        RP_Params.OrbitType = EOrbitType::PARABOLA;
        RP_Params.Period = 0;
        RP_Params.A = 0;
    }

    // E > 1, Hyperbola
    else
    {
        struct MySplinePoint
        {
            MySplinePoint() {}
            MySplinePoint(FVector InPosition) : Position(InPosition) {}
            MySplinePoint(FVector InPosition, FVector InArriveTangent, FVector InLeaveTangent)
                : Position(InPosition)
                , ArriveTangent(InArriveTangent)
                , LeaveTangent(InLeaveTangent) 
            {}
            FVector Position = FVector::Zero();
            FVector ArriveTangent = FVector::Zero();
            FVector LeaveTangent = FVector::Zero();
        };
        
        std::list<MySplinePoint> Points;
        const float ESquared = pow(Eccentricity, 2);
        const float EReduced = sqrt(ESquared - 1);
        RP_Params.A = RP_Params.P / (1 - ESquared); // Params.A < 0
        const float A = -RP_Params.A; // A > 0
        const float Periapsis = RP_Params.P / (Eccentricity + 1);
        const FVector VecVertical = VecHNorm.Cross(VecENorm);
        const FVector VecHorizontal = -VecENorm;
        constexpr int MAX_POINTS = 20;
        //const float MAX = sqrt((pow(Physics.WorldRadius, 2) + (pow(Eccentricity, 2) - 1.) * pow(RP_Params.A, 2)) / pow(Eccentricity, 2));
        //const float Delta = 2. * MAX / (pow(MAX_POINTS / 2 - 1, 3) / 3.);
        // DeltaX value based on a steep hyperbola
        const float DeltaX = Physics.WorldRadius / pow(2, MAX_POINTS / 2 - 2);
        //const float DeltaY =
        //      sqrt((pow(Physics.WorldRadius, 2) + pow(RP_Params.P, 2)) / (ESquared - 1))
        //    / pow(2, MAX_POINTS / 2);
        //const float Delta = std::min(DeltaX, DeltaY);

        Points.emplace_front(-Periapsis * VecHorizontal + Physics.VecF1);
        for(int i = 0; i < MAX_POINTS / 2; i++)
        {
            const float X = pow(2, i) * DeltaX;
            const FVector VecX = X * VecHorizontal;

            const float Y = EReduced * sqrt(pow(X + A, 2) - pow(A, 2));
            const FVector VecY = VecVertical * Y;

            const FVector P1 = VecX - Periapsis * VecHorizontal + VecY + Physics.VecF1;
            const FVector P2 = VecX - Periapsis * VecHorizontal - VecY + Physics.VecF1;

            if(i == 0)
            {
                // tangent slope is EReduces/-EReduced, respectively
                const FVector VecF1Norm = (VecHorizontal + EReduced * VecVertical) / Eccentricity;
                const FVector VecF2Norm = (VecHorizontal - EReduced * VecVertical) / Eccentricity;
                
                // base change matrix from (VecHorizontal, VecVertical) to (F1, F2)
                const FMatrix2x2 BaseChange = FMatrix2x2
                    ( Eccentricity / 2., Eccentricity / 2.
                    , Eccentricity / 2. / EReduced, -Eccentricity / 2. / EReduced);
                
                const FVector2f FA = BaseChange.TransformVector(FVector2f(A + X, Y));
                const FVector TangentA = (VecF1Norm * FA.X - VecF2Norm * FA.Y).GetSafeNormal() * RP_Params.P;
                Points.emplace_back (P1, TangentA * 2., TangentA);

                const FVector2f FB = BaseChange.TransformVector(FVector2f(A + X, -Y));
                const FVector TangentB = (VecF1Norm * FB.X - VecF2Norm * FB.Y).GetSafeNormal() * RP_Params.P;
                Points.emplace_front(P2, TangentB * 2., TangentB);
            }
            else
            {
                Points.emplace_back(P1);
                Points.emplace_front(P2);
            }
            if(P1.Length() > Physics.WorldRadius)
            {
                break;
            }
        }

        RP_SplinePoints.Empty();
        RP_SplinePoints.Reserve(Points.size());
        const int32 NumPoints = Points.size();
        for(int i = 0; i < NumPoints; i++)
        {
            MySplinePoint Point = Points.front();
            if(Point.ArriveTangent.IsZero())
            {
                RP_SplinePoints.Emplace(i, Point.Position);
            }
            else
            {
                RP_SplinePoints.Emplace(i, Point.Position, Point.ArriveTangent, Point.LeaveTangent);
            }
            Points.pop_front();
        }
        AddPointsToSpline();

        Spline->SetClosedLoop(false, false);
        RP_Params.OrbitType = EOrbitType::HYPERBOLA;
        RP_Params.Period = 0;
    }
    Spline->UpdateSpline();

    if(RP_Params.OrbitType != EOrbitType::LINEBOUND)
    {
        SplineDistance = Spline->GetDistanceAlongSplineAtSplineInputKey(Spline->FindInputKeyClosestToWorldLocation(VecR));
    }
    // else
    // `SplineKey` is not needed,
    // `SplineDistance` and `DistanceZero` are set already

    TArray<USceneComponent*> Meshes;
    SplineMeshParent->GetChildrenComponents(false, Meshes);
    for(auto Mesh : Meshes)
    {
        Mesh->DestroyComponent();
    }

    if(bTrajectoryShowSpline)
    {
#if WITH_EDITOR
        if(GetWorld()->WorldType != EWorldType::Editor || bIsVisibleInEditor)
#endif
        SpawnSplineMesh
            ( Cast<IHasOrbitColor>(RP_Body)->GetOrbitColor()
            , ESplineMeshParentSelector::Permanent
            , InstanceUI
            );
    }
}

void AOrbit::UpdateControllParams(FPhysics Physics)
{
    const FVector VecRKepler = GetVecRKepler(Physics);
    ControllParams.VecH = VecRKepler.Cross(VecVelocity);
    ControllParams.P = ControllParams.VecH.SquaredLength() / Physics.Alpha;
    ControllParams.Energy = pow(ScalarVelocity, 2) / 2. - Physics.Alpha / RKepler;
    ControllParams.VecE = UFunctionLib::Eccentricity(VecRKepler, VecVelocity, Physics.Alpha);
}

void AOrbit::UpdateSplineMeshScale(float InScaleFactor)
{
    SplineMeshScaleFactor = InScaleFactor;
    TArray<USceneComponent*> SceneComponents;
    SplineMeshParent->GetChildrenComponents(false, SceneComponents);
    for(USceneComponent* SceneComponent  : SceneComponents)
    {
        USplineMeshComponent* Mesh = Cast<USplineMeshComponent>(SceneComponent);
        UMaterialInstanceDynamic* Material = Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(0));
        Material->SetScalarParameterValue("NumBands", 4. / InScaleFactor);
        
        Mesh->SetStartScale(SplineMeshScaleFactor * FVector2D::UnitVector, false);
        Mesh->SetEndScale(SplineMeshScaleFactor * FVector2D::UnitVector, false);
        Mesh->UpdateMesh();
    }
}

void AOrbit::AddPointsToSpline()
{
    Spline->ClearSplinePoints();
    
    const FVector Loc = Spline->GetComponentLocation(); // should be (0, 0, 0)
    for(FSplinePoint& p : RP_SplinePoints)
    {
        p.Position -= Loc;
    }
    Spline->AddPoints(RP_SplinePoints, false);
}

bool AOrbit::GetVisibility(const FInstanceUI& InstanceUI) const
{

    bool bIsVisibleInEditorActive = false;
#if WITH_EDITOR
    if(GetWorld()->WorldType == EWorldType::Editor)
    {
        bIsVisibleInEditorActive = bIsVisibleInEditor;
    }
#endif
    
    return
           bIsVisibleInEditorActive
        || bIsVisibleMouseover
        || bIsVisibleShowMyTrajectory != bIsVisibleToggleMyTrajectory
        || bIsVisibleAccelerating
        || InstanceUI.bShowAllTrajectories
        || InstanceUI.Selected.Orbit == this;
}

FString AOrbit::GetParamsString()
{
    FString StrOrbitType;
    switch(RP_Params.OrbitType)
    {
    case EOrbitType::CIRCLE:
        StrOrbitType = TEXT("Circle");
        break;
    case EOrbitType::ELLIPSE:
        StrOrbitType = TEXT("Ellipse");
        break;
    case EOrbitType::PARABOLA:
        StrOrbitType = TEXT("Parabola");
        break;
    case EOrbitType::HYPERBOLA:
        StrOrbitType = TEXT("Hyperbola");
        break;
    case EOrbitType::LINEBOUND:
        StrOrbitType = TEXT("LineBound");
        break;
    case EOrbitType::LINEUNBOUND:
        StrOrbitType = TEXT("LineUnbound");
        break;
    }
    return StrOrbitType + FString::Printf
        (TEXT(", E = (%.2f, %.2f, %.2f), H = (%.1f, %.1f, %.1f), P = %.1f, Energy = %.1f, Period = %.1f, A = %.1f")
        , RP_Params.VecE.X
        , RP_Params.VecE.Y
        , RP_Params.VecE.Z
        , RP_Params.VecH.X
        , RP_Params.VecH.Y
        , RP_Params.VecH.Z
        , RP_Params.P
        , RP_Params.Energy
        , RP_Params.Period
        , RP_Params.A
        );
}

FVector AOrbit::GetCircleVelocity(FPhysics Physics)
{
    const FVector VecRKepler = GetVecRKepler(Physics);
    RKepler = VecRKepler.Length();
    return FVector(0., 0., sqrt(Physics.Alpha / RKepler) / RKepler).Cross(VecRKepler);
}

void AOrbit::SetReadyFlags(EOrbitReady ReadyFlags)
{
    OrbitReady |= ReadyFlags;
    if(OrbitReady == EOrbitReady::All)
    {
        UE_LOG(LogMyGame, Display, TEXT("%s: Orbit ready"), *GetFullName())
        Initialize();
    }
    else
    {
        UE_LOG
            ( LogMyGame
            , Display
            , TEXT("%s: Internal %s, Body %s")
            , *GetFullName()
            , !(OrbitReady & EOrbitReady::InternalReady  ) ? TEXT("waiting") : TEXT("ready")
            , !(OrbitReady & EOrbitReady::BodyReady      ) ? TEXT("waiting") : TEXT("ready")
            )
    }
}

void AOrbit::OnRep_OrbitState()
{
    SplineDistance = RP_OrbitState.SplineDistance;
}

void AOrbit::OnRep_Body()
{
    SetEnableVisibility(RP_Body->Implements<UHasOrbitColor>());
    SetReadyFlags(EOrbitReady::BodyReady);
}

void AOrbit::HandleBeginMouseOver(AActor* Actor)
{
    GEngine->GetEngineSubsystem<UMyState>()->WithInstanceUI(this, [this] (FInstanceUI& InstanceUI)
    {
        float Size = Cast<IHasMesh>(RP_Body)->GetMesh()->Bounds.SphereRadius;
        InstanceUI.Hovered = {this, Size };
        bIsVisibleMouseover = true;
        UpdateVisibility(InstanceUI);
    });
}

void AOrbit::HandleEndMouseOver(AActor* Actor)
{
    GEngine->GetEngineSubsystem<UMyState>()->WithInstanceUI(this, [this] (FInstanceUI& InstanceUI)
    {
        InstanceUI.Hovered.Orbit = nullptr;
        bIsVisibleMouseover = false;
        UpdateVisibility(InstanceUI);
    });
}

void AOrbit::HandleClick(AActor*, FKey Button)
{
    if(Button == EKeys::LeftMouseButton)
    {
        GEngine->GetEngineSubsystem<UMyState>()->WithInstanceUI(this, [this] (FInstanceUI& InstanceUI)
        {
            AOrbit* Orbit = InstanceUI.Selected.Orbit;
            if(IsValid(Orbit))
            {
                InstanceUI.Selected.Orbit = nullptr;
                Orbit->UpdateVisibility(InstanceUI);
            }
            if(Orbit != this)
            {
                float Size = Cast<IHasMesh>(RP_Body)->GetMesh()->Bounds.SphereRadius;
                InstanceUI.Selected = {this, Size };
            }
            UpdateVisibility(InstanceUI);
        });
    }
}

void AOrbit::SetInitialParams(FVector VecV, FPhysics Physics)
{
    // store initial position of orbit body
    VecRZero = GetVecR();
    
    RP_Params.VecH = GetVecRKepler(Physics).Cross(VecV);
    RP_Params.VecE = VecV.Cross(RP_Params.VecH) / Physics.Alpha - GetVecRKepler(Physics).GetSafeNormal();
}

void AOrbit::UpdateVisibility(const FInstanceUI& InstanceUI)
{
    SplineMeshParent->SetVisibility(GetVisibility(InstanceUI), true);
}

void AOrbit::SpawnSplineMesh(FLinearColor Color, ESplineMeshParentSelector ParentSelector, FInstanceUI InstanceUI)
{
    const int nIndices = static_cast<int>(round(Spline->GetSplineLength() / SplineMeshLength));

    if(!StaticMesh)
    {
        UE_LOG
        ( LogMyGame
        , Warning
        , TEXT("%s: AOrbit::SpawnSplineMesh: SMSplineMesh null, skipping")
        , *GetFullName()
        )
        return;
    }
    if(!SplineMeshMaterial)
    {
        UE_LOG
        ( LogMyGame
        , Warning
        , TEXT("%s: AOrbit::SpawnSplineMesh: MSplineMesh null, skipping")
        , *GetFullName()
        )
        return;
    }
    
    if(nIndices >= 2)
    {
        std::vector<int> Indices(nIndices);
        std::iota(Indices.begin(), Indices.end(), 0);

        if(Spline->IsClosedLoop() && RP_Params.OrbitType != EOrbitType::LINEBOUND)
        {
            Indices.push_back(0);
        }
        for(int i = 0; i < Indices.size() - 1; i++)
        {
            USceneComponent* Parent = nullptr;
            switch(ParentSelector)
            {
            case ESplineMeshParentSelector::Temporary:
                Parent = TemporarySplineMeshParent;
                break;
            case ESplineMeshParentSelector::Permanent:
                Parent = SplineMeshParent;
                break;
            }
            
            USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);
            
            SplineMesh->SetMobility(EComponentMobility::Stationary);
            SplineMesh->SetVisibility(GetVisibility(InstanceUI));
            
            // if I dont register here, the spline mesh doesn't render
            SplineMesh->RegisterComponent();
            // in theory, I could use `SetupAttachment` and then `RegisterComponent`
            // in practice, that messes up the location (i.e. it moves the spline mesh)
            // and the spline meshes do not show up with their correct names in the editor
            SplineMesh->AttachToComponent(Parent, FAttachmentTransformRules::KeepWorldTransform);
            // if I don't add instance here, the spline meshes don't show in the component list in the editor
            AddInstanceComponent(SplineMesh);

            SplineMesh->CastShadow = false;
            SplineMesh->SetStaticMesh(StaticMesh);

            UMaterialInstanceDynamic* DynamicMaterial =
                SplineMesh->CreateDynamicMaterialInstance(0, SplineMeshMaterial);
            DynamicMaterial->SetVectorParameterValue("StripesColor", Color);

            const FVector VecStartPos =
                Spline->GetLocationAtDistanceAlongSpline(Indices[i] * SplineMeshLength, ESplineCoordinateSpace::World);
            const FVector VecStartDirection =
                Spline->GetTangentAtDistanceAlongSpline
                    ( Indices[i] * SplineMeshLength
                    , ESplineCoordinateSpace::World
                    ).GetUnsafeNormal() * SplineMeshLength;
            const FVector VecEndPos =
                Spline->GetLocationAtDistanceAlongSpline(Indices[i + 1] * SplineMeshLength, ESplineCoordinateSpace::World);
            const FVector VecEndDirection =
                Spline->GetTangentAtDistanceAlongSpline
                    ( Indices[i + 1] * SplineMeshLength
                    , ESplineCoordinateSpace::World
                    ).GetUnsafeNormal() * SplineMeshLength;
            SplineMesh->SetStartAndEnd
                ( VecStartPos
                , VecStartDirection
                , VecEndPos
                , VecEndDirection
                , false
                );
            SplineMesh->SetStartScale(SplineMeshScaleFactor * FVector2D::UnitVector, false);
            SplineMesh->SetEndScale(SplineMeshScaleFactor * FVector2D::UnitVector, false);
            SplineMesh->UpdateMesh();
        }
    }
}

#if WITH_EDITOR
void AOrbit::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
    Super::PostEditChangeChainProperty(PropertyChangedEvent);

    const FName Name = PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName();

    static const FName FNameBTrajectoryShowSpline = GET_MEMBER_NAME_CHECKED(AOrbit, bTrajectoryShowSpline);
    static const FName FNameVisibleInEditor       = GET_MEMBER_NAME_CHECKED(AOrbit, bIsVisibleInEditor   );
    static const FName FNameBVisibleMouseOver     = GET_MEMBER_NAME_CHECKED(AOrbit, bIsVisibleMouseover  );
    static const FName FNameBVisibleShowMy        = GET_MEMBER_NAME_CHECKED(AOrbit, bIsVisibleShowMyTrajectory);
    static const FName FNameBVisibleToggleMy      = GET_MEMBER_NAME_CHECKED(AOrbit, bIsVisibleToggleMyTrajectory);
    static const FName FNameSplineMeshLength      = GET_MEMBER_NAME_CHECKED(AOrbit, SplineMeshLength     );
    static const FName FNameVelocity              = GET_MEMBER_NAME_CHECKED(AOrbit, ScalarVelocity       );
    static const FName FNameVelocityVCircle       = GET_MEMBER_NAME_CHECKED(AOrbit, VelocityVCircle      );
    static const FName FNameVecVelocity           = GET_MEMBER_NAME_CHECKED(AOrbit, VecVelocity          );
    static const FName FNameSMTrajectory          = GET_MEMBER_NAME_CHECKED(AOrbit, StaticMesh           );
    static const FName FNameSplineMeshMaterial    = GET_MEMBER_NAME_CHECKED(AOrbit, SplineMeshMaterial   );
    static const FName FNameSplineMeshScaleFactor = GET_MEMBER_NAME_CHECKED(AOrbit, SplineMeshScaleFactor);

    if(
           Name == FNameVisibleInEditor
        || Name == FNameBVisibleMouseOver
        || Name == FNameBVisibleShowMy
        || Name == FNameBVisibleToggleMy
        )
    {
        UpdateVisibility(InstanceUIEditorDefault);
    }
    else if
        (  Name == FNameSplineMeshLength
        || Name == FNameBTrajectoryShowSpline
        || Name == FNameSMTrajectory
        || Name == FNameSplineMeshMaterial
        || Name == FNameSplineMeshScaleFactor
        )
    {
    }
    else if
        (  Name == FNameVelocity
        || Name == FNameVelocityVCircle
        || Name == FNameVecVelocity
        )
    {
        const FVector VelocityNormal = VecVelocity.GetSafeNormal(1e-8, FVector(0., 1., 0.));

        if(Name == FNameVelocity)
        {
            SetInitialParams(ScalarVelocity * VelocityNormal, PhysicsEditorDefault);
        }
        else if(Name == FNameVelocityVCircle)
        {
            SetInitialParams(GetCircleVelocity(PhysicsEditorDefault).Length() * VelocityVCircle * VelocityNormal, PhysicsEditorDefault);
        }
        else if(Name == FNameVecVelocity)
        {
            SetInitialParams(VecVelocity, PhysicsEditorDefault);
        }
        bIsVisibleInEditor = false;
    }
    
    if(IsValid(RP_Body))
    {
        Update(FVector::Zero(), PhysicsEditorDefault, InstanceUIEditorDefault);
    }
    else
    {
        UE_LOG
            ( LogMyGame
            , Warning
            , TEXT("%s: PostEditChangedChainProperty: body invalid")
            , *GetFullName()
            )
    }
}
#endif

void AOrbit::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AOrbit, RP_bClosedLoop )

    DOREPLIFETIME(AOrbit, RP_Params      )
    DOREPLIFETIME(AOrbit, RP_SplinePoints)
    
    DOREPLIFETIME_CONDITION(AOrbit, RP_Body, COND_InitialOnly)

    // TODO: if actor location replicates once in the beginning, only line orbits need to freeze the spline key
    // logically, 'RP_OrbitState' should replicate only once with 'COND_InitialOnly'
    // however, I call 'FreezeOrbitState' in 'AcknowledgePossession', way after initial replication
    // thus, the replicated orbit state is the variable default
    // I can move 'FreezeOrbitState' to some earlier event, but I like the idea that all kind of loading has been
    // finished when the orbit state is finally replicated - to the effect that lag is minimized
    // Two consequences:
    // 1. The HUD of a client is initialized with an orbit state in its default value (not a problem)
    // 2. When a third player joins, the first client re-initializes his orbit, too. Should be fine.
    DOREPLIFETIME(AOrbit, RP_OrbitState )
}

void AOrbit::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
    Super::PreReplication(ChangedPropertyTracker);

    // TODO: test performance impact, ideally this isn't necessary
    DOREPLIFETIME_ACTIVE_OVERRIDE(AOrbit, RP_Params      , !bIsChanging)
    DOREPLIFETIME_ACTIVE_OVERRIDE(AOrbit, RP_SplinePoints, !bIsChanging)
}