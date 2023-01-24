#include "Actors/Orbit.h"

#include <numeric>

#include "Actors/MyCharacter.h"
#include "Components/SplineMeshComponent.h"
#include "HUD/MyHUD.h"
#include "Lib/FunctionLib.h"
#include "Logging/LogMacros.h"
#include "Modes/MyPlayerController.h"
#include "Modes/MyGameState.h"
#include "Modes/MyGameInstance.h"
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
            GetOrbit()->Update(Physics, InstanceUI);
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
    //Root->SetMobility(EComponentMobility::Stationary);
    SetRootComponent(Root);

    Spline = CreateDefaultSubobject<USplineComponent>("Orbit");
    //Spline->SetMobility(EComponentMobility::Stationary);
    Spline->SetupAttachment(Root);
    Spline->SetIsReplicated(true);

    SplineMeshParent = CreateDefaultSubobject<USceneComponent>("SplineMeshes");
    SplineMeshParent->SetupAttachment(Root);
    //SplineMeshParent->SetMobility(EComponentMobility::Stationary);

    TemporarySplineMeshParent = CreateDefaultSubobject<USceneComponent>("TemporarySplineMeshes");
    TemporarySplineMeshParent->SetupAttachment(Root);
    //TemporarySplineMeshParent->SetMobility(EComponentMobility::Stationary);
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
    const auto* GI = GetGameInstance<UMyGameInstance>();
    const auto* GS = GetWorld()->GetGameState<AMyGameState>();
    const auto InstanceUI = MyState->GetInstanceUI(GI);
    const auto Physics = MyState->GetPhysics(GS);

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
    
    if(RP_Body->GetLocalRole() == ROLE_AutonomousProxy)
    {
        Cast<AMyCharacter>(RP_Body)->GetController<AMyPlayerController>()->GetHUD<AMyHUD>()->SetReadyFlags(EHUDReady::OrbitReady);
        if(!RP_OrbitState.VecR.IsZero())
        {
            VecVelocity = RP_OrbitState.VecVelocity;
            RP_Body->SetActorLocation(RP_OrbitState.VecR);
        }
    }
    // make sure the orbit has the game physics, instead of the editor default physics from construction
    Update(Physics, InstanceUI);

    bIsInitialized = true;
    SetActorTickEnabled(true);
}

void AOrbit::CorrectSplineLocation()
{
    SetActorLocation(
        GetVecR() - Spline->GetLocationAtSplineInputKey(
                Spline->FindInputKeyClosestToWorldLocation(GetVecR())
            , ESplineCoordinateSpace::World)
        );
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

    const auto* GS = GetWorld()->GetGameState<AMyGameState>();
    const auto* GI = GetGameInstance<UMyGameInstance>();
    auto* MyState = GEngine->GetEngineSubsystem<UMyState>();
    const auto Physics = MyState->GetPhysics(GS);
    const FVector VecRKepler = GetVecRKepler(Physics);

    FVector NewVecRKepler;
    if(RKepler < 100.)
    {
        // newtonian motion isn't stable here because of the 1/r potential
        NewVecRKepler = Spline->GetLocationAtDistanceAlongSpline(
            Spline->GetDistanceAlongSplineAtSplineInputKey(
                Spline->FindInputKeyClosestToWorldLocation(
                    VecRKepler           
                    )
                ) + ScalarVelocity * DeltaTime
            , ESplineCoordinateSpace::World
            );
        VecVelocity =UFunctionLib::VecVelocity
            ( Params.VecE
            , NewVecRKepler
            , Params.VecH
            , Physics.Alpha
            , (NewVecRKepler - VecRKepler) / DeltaTime // not accurate
            );
    }
    else
    {
        // newtonian motion
        NewVecRKepler = VecRKepler + VecVelocity * DeltaTime
            + pow(DeltaTime, 2) * Physics.Alpha / VecRKepler.SquaredLength() * -VecRKepler.GetSafeNormal();
        VecVelocity = (NewVecRKepler - VecRKepler) / DeltaTime;
    }
    
    ScalarVelocity = VecVelocity.Length();
    VelocityVCircle = ScalarVelocity / GetCircleVelocity(Physics).Length();
    RKepler = NewVecRKepler.Length();
    
    RP_Body->SetActorLocation(NewVecRKepler + Physics.VecF1);
    
    UpdateControllParams(Physics);
}

void AOrbit::Update(FVector DeltaVecV, FPhysics Physics, FInstanceUI InstanceUI)
{
    const FVector VecR = GetVecR();
    
    // transform location vector r to Kepler coordinates, where F1 is the origin
    const FVector VecRKepler = GetVecRKepler(Physics);

    VecVelocity += DeltaVecV;
    Params.VecH = VecRKepler.Cross(VecVelocity);
    Params.VecE = UFunctionLib::Eccentricity(VecRKepler, VecVelocity, Physics.Alpha);
    Params.P = Params.VecH.SquaredLength() / Physics.Alpha;

    // not necessary, as the velocity variables are set by the Tick
    // but if not set here, the editor doesn't show the current velocity
    ScalarVelocity = VecVelocity.Length();
    VelocityVCircle = ScalarVelocity / GetCircleVelocity(Physics).Length();
    RKepler = VecRKepler.Length();
    
    Params.Energy = pow(ScalarVelocity, 2) / 2. - Physics.Alpha / RKepler;
    
    Params.Eccentricity = Params.VecE.Length();
    const FVector VecENorm = Params.VecE.GetSafeNormal();
    const FVector VecHNorm = Params.VecH.GetSafeNormal();

    // the energy of the weakest bound state: a circular orbit at R_MAX
    const double E_BOUND_MIN = -Physics.Alpha / (2 * Physics.WorldRadius);

    // TODO testing
    if  (  Params.VecH.Length() / RKepler < 1.
        && Params.Eccentricity > 1.0 - EccentricityTolerance
        && Params.Eccentricity < 1.0 + EccentricityTolerance
        )
    {
        const double EMIN = -Physics.Alpha / Physics.WorldRadius;
        const double E = VecVelocity.SquaredLength() / 2. - Physics.Alpha / RKepler;

        // bound
        if(E < EMIN)
        {
            const double Apsis = -Physics.Alpha / E;
            const FVector VecVNorm = VecVelocity.GetSafeNormal();
            if(VecVNorm.IsZero())
            {
                SplinePoints =
                    { FSplinePoint(0, VecR, ESplinePointType::Linear)
                    , FSplinePoint(1, -VecRKepler + Physics.VecF1, ESplinePointType::Linear)
                    };
                AddPointsToSpline();
            }
            else
            {
                SplinePoints =
                    { FSplinePoint(0, -VecVNorm * Apsis + Physics.VecF1, ESplinePointType::Linear)
                    , FSplinePoint(1,  VecVNorm * Apsis + Physics.VecF1, ESplinePointType::Linear)
                    };
                AddPointsToSpline();
            }
            
            Spline->SetClosedLoop(true, false);
            
            Params.OrbitType = EOrbitType::LINEBOUND;
            Params.A = UFunctionLib::SemiMajorAxis(VecRKepler, VecVelocity, Physics.Alpha);
            Params.Period = UFunctionLib::PeriodEllipse(Params.A, Physics.Alpha);
        }

        // unbound
        else
        {
            SplinePoints =
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
            Params.OrbitType = EOrbitType::LINEUNBOUND;
            Params.A = -Physics.Alpha / 2 / Params.Energy;
            Params.Period = 0;
        }
    }
    
    // E = 0, circle
    else if(VecENorm.IsZero())
    {
        const FVector VecP2 = VecHNorm.Cross(VecRKepler);
        const FVector VecT1 = VecHNorm.Cross(VecRKepler) * SplineToCircle;
        const FVector VecT4 = VecRKepler * SplineToCircle;
        
        SplinePoints =
            { FSplinePoint(0,  VecR,  VecT1,  VecT1)
            , FSplinePoint(1, VecP2 + Physics.VecF1, -VecT4, -VecT4)
            , FSplinePoint(2, -VecRKepler + Physics.VecF1 , -VecT1, -VecT1)
            , FSplinePoint(3, -VecP2 + Physics.VecF1,  VecT4,  VecT4)
            };
        AddPointsToSpline();
        Spline->SetClosedLoop(true, false);
        Params.OrbitType = EOrbitType::CIRCLE;
        Params.A = RKepler;
        Params.Period = UFunctionLib::PeriodEllipse(RKepler, Physics.Alpha);
    }

    // 0 < E < 1, Ellipse
    // TODO: check if still true for low tolerance
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
    else if(Params.Eccentricity <= 1. - EccentricityTolerance || (Params.Eccentricity <= 1 && Params.Energy < E_BOUND_MIN))
    {
        std::list<FVector> Points;
        Params.A = Params.P / (1. - pow(Params.Eccentricity, 2));
        const double B = Params.A * sqrt(1 - pow(Params.Eccentricity, 2));
        const FVector VecVertical = Params.VecH.Cross(Params.VecE).GetSafeNormal();
        const FVector VecC = Physics.VecF1 - Params.A * Params.VecE;

        const FVector Vertex1 = Params.A * (1. - Params.Eccentricity) * VecENorm + Physics.VecF1;
        Points.emplace_front(Vertex1);

        bool bEllipseIsCut;
        double TMax; // eccentric anomaly t
        if(Params.A * (1. + Params.Eccentricity) > Physics.WorldRadius)
        {
            bEllipseIsCut = true;
            TMax = acos((1. - Physics.WorldRadius / Params.A) / Params.Eccentricity);
            Spline->SetClosedLoop(false, false);
        }
        else
        {
            bEllipseIsCut = false;
            TMax = PI;   
            Spline->SetClosedLoop(true, false);
        }

        const int IMax = 32;
        const double DeltaT = TMax / (IMax / 2);
        for(int i = 1; i < IMax / 2; i++)
        {
            const double T = i * DeltaT;
            const FVector VecX = Params.A * cos(T) * VecENorm;
            const FVector VecY = B * sin(T) * VecVertical;
            Points.emplace_front(VecC + VecX + VecY + Physics.VecF1);
            Points.emplace_back (VecC + VecX - VecY + Physics.VecF1);
        }
        if(bEllipseIsCut)
        {
            const FVector VecX = Params.A * cos(TMax) * VecENorm;
            const FVector VecY = B * sin(TMax) * VecVertical;
            Points.emplace_front(VecC + VecX + VecY + Physics.VecF1);
            Points.emplace_back (VecC + VecX - VecY + Physics.VecF1);
        }
        else
        {
            Points.emplace_front(Params.A * (1. + Params.Eccentricity) * -VecENorm);
        }
        
        SplinePoints.Empty();
        SplinePoints.Reserve(Points.size());
        int InputKey = 0;
        for(std::list<FVector>::const_iterator IPoint = Points.begin(); IPoint != Points.end(); ++IPoint)
        {
            const FVector P = *IPoint;
            SplinePoints.Emplace (InputKey, P);
            InputKey++;
        }
        AddPointsToSpline();
        Params.OrbitType = EOrbitType::ELLIPSE;
        Params.Period = UFunctionLib::PeriodEllipse(Params.A, Physics.Alpha);
    }
    
    // E = 1, Parabola
    else if(Params.Eccentricity <= 1. + EccentricityTolerance)
    {
        std::list<FVector> Points;
        const FVector VecHorizontal = VecHNorm.Cross(VecENorm);
        constexpr int MAX_POINTS = 20;
        const double Delta = sqrt(2 * Physics.WorldRadius / Params.P) / ((MAX_POINTS - 1) / 2);

        Points.emplace_front(VecENorm * Params.P / (1. + Params.Eccentricity) + Physics.VecF1);
        for(int i = 1; i < MAX_POINTS / 2; i++)
        {
            const FVector VecX = i * Delta * VecHorizontal * Params.P;
            const FVector VecY = VecENorm / 2. * (1 - pow(i * Delta, 2)) * Params.P;
            Points.emplace_back(VecY + VecX + Physics.VecF1);
            Points.emplace_front(VecY - VecX + Physics.VecF1);
        }

        SplinePoints.Empty();
        SplinePoints.Reserve(Points.size());
        const int32 NumPoints = Points.size();
        for(int i = 0; i < NumPoints; i++)
        {
            SplinePoints.Emplace(i, Points.front());
            Points.pop_front();
        }
        AddPointsToSpline();

        // ClearSplinePoints();
        // for(const FVector Point : Points)
        // {
        // 	AddSplineWorldPoint(Point);
        // }
        
        Spline->SetClosedLoop(false, false);
        Params.OrbitType = EOrbitType::PARABOLA;
        Params.Period = 0;
        Params.A = 0;
    }

    // E > 1, Hyperbola
    else
    {
        std::list<FVector> Points;
        const FVector VecVertical = VecHNorm.Cross(VecENorm);
        
        constexpr int MAX_POINTS = 32;
        
        // Periapsis
        const double Periapsis = Params.P / (Params.Eccentricity + 1);
        const double DeltaR = (Physics.WorldRadius - Periapsis) / (pow(MAX_POINTS / 2, 2));
        Points.emplace_front(Periapsis * VecENorm + Physics.VecF1);
        
        for(int i = 1; i <= MAX_POINTS / 2; i++)
        {
            const double R = Periapsis + pow(i, 2) * DeltaR;
            const double T = acos((Params.P / R - 1.) / Params.Eccentricity);
            const FVector VecX = R * cos(T) * VecENorm;
            const FVector VecY = R * sin(T) * VecVertical;

            Points.emplace_front(VecX + VecY + Physics.VecF1);
            Points.emplace_back(VecX - VecY + Physics.VecF1);
        }

        SplinePoints.Empty();
        SplinePoints.Reserve(Points.size());
        int InputKey = 0;
        for(std::list<FVector>::const_iterator IPoint = Points.begin(); IPoint != Points.end(); ++IPoint)
        {
            if(abs(InputKey - MAX_POINTS / 2) == 1)
            {
                const auto VecT = -UFunctionLib::VecVelocity(Params.VecE, *IPoint, Params.VecH, Physics.Alpha, FVector::Zero()).GetSafeNormal();
                double ArriveTangentLength = (*std::prev(IPoint) - *IPoint).Length();
                double LeaveTangentLength  = (*std::next(IPoint) - *IPoint).Length();
                //RP_SplinePoints.Emplace(InputKey, *IPoint, VecT * ArriveTangentLength, VecT * LeaveTangentLength);
                SplinePoints.Emplace(InputKey, *IPoint);
            }
            else
            {
                SplinePoints.Emplace(InputKey, *IPoint);
            }
            InputKey++;
        }
        AddPointsToSpline();

        Spline->SetClosedLoop(false, false);
        Params.A = Params.P / (1 - pow(Params.Eccentricity, 2)); // Params.A < 0
        Params.OrbitType = EOrbitType::HYPERBOLA;
        Params.Period = 0;
    }
    Spline->UpdateSpline();

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

void AOrbit::Update(FPhysics Physics, FInstanceUI InstanceUI)
{
    Update(FVector::Zero(), Physics, InstanceUI);
}

void AOrbit::UpdateControllParams(FPhysics Physics)
{
    const FVector VecRKepler = GetVecRKepler(Physics);
    ControllParams =
        { UFunctionLib::Eccentricity(VecRKepler, VecVelocity, Physics.Alpha).Length()
        , VecRKepler.Cross(VecVelocity).SquaredLength() / Physics.Alpha
        ,pow(ScalarVelocity, 2) / 2. - Physics.Alpha / RKepler
        };
}

void AOrbit::UpdateSplineMeshScale(double InScaleFactor)
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
    for(FSplinePoint& p : SplinePoints)
    {
        p.Position -= Loc;
    }
    Spline->AddPoints(SplinePoints, false);
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
    switch(Params.OrbitType)
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
        (TEXT(", E = (%.2f, %.2f, %.2f) = %.3f, H = (%.1f, %.1f, %.1f), P = %.1f, Energy = %.1f, Period = %.1f, A = %.1f")
        , Params.VecE.X
        , Params.VecE.Y
        , Params.VecE.Z
        , Params.Eccentricity
        , Params.VecH.X
        , Params.VecH.Y
        , Params.VecH.Z
        , Params.P
        , Params.Energy
        , Params.Period
        , Params.A
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
    if(bIsInitialized)
    {
        VecVelocity = RP_OrbitState.VecVelocity;
        RP_Body->SetActorLocation(RP_OrbitState.VecR);

        UMyState* MyState = GEngine->GetEngineSubsystem<UMyState>();
        const auto* GI = GetGameInstance<UMyGameInstance>();
        const auto* GS = GetWorld()->GetGameState<AMyGameState>();
        const auto InstanceUI = MyState->GetInstanceUI(GI);
        const auto Physics = MyState->GetPhysics(GS);
        Update(Physics, InstanceUI);
    }
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
        double Size = Cast<IHasMesh>(RP_Body)->GetMesh()->Bounds.SphereRadius;
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
                double Size = Cast<IHasMesh>(RP_Body)->GetMesh()->Bounds.SphereRadius;
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
    VecVelocity = VecV;   
}

void AOrbit::UpdateVisibility(const FInstanceUI& InstanceUI)
{
    const bool bVisibility = GetVisibility(InstanceUI);
    SplineMeshParent->SetVisibility(bVisibility, true);
}

void AOrbit::SpawnSplineMesh
    ( FLinearColor Color
    , ESplineMeshParentSelector ParentSelector
    , FInstanceUI InstanceUI
    )
{
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

    std::vector<int32> Indices(Spline->GetNumberOfSplinePoints());
    std::iota(Indices.begin(), Indices.end(), 0);
    if(Spline->IsClosedLoop() && Params.OrbitType != EOrbitType::LINEBOUND)
    {
        Indices.push_back(0);
    }

    for(int32 i = 0; i < Indices.size() - 1; i++)
    {
        const FVector VecStartPos = Spline->GetLocationAtSplinePoint(Indices[i], ESplineCoordinateSpace::World);
        const FVector VecEndPos = Spline->GetLocationAtSplinePoint(Indices[i + 1], ESplineCoordinateSpace::World);
        
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
        DynamicMaterial->SetScalarParameterValue("StripesLength", 500. + UFunctionLib::VelocityInfinity(Params.Energy));

        const FVector VecStartDirection =
            Spline->GetTangentAtSplinePoint(Indices[i], ESplineCoordinateSpace::World);
        const FVector VecEndDirection =
            Spline->GetTangentAtSplinePoint(Indices[i + 1], ESplineCoordinateSpace::World);
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
        DistanceToOrbit =
            ( Spline->GetLocationAtDistanceAlongSpline(
                Spline->GetDistanceAlongSplineAtSplineInputKey(
                    Spline->FindInputKeyClosestToWorldLocation( GetVecR() )
                ), ESplineCoordinateSpace::World
            ) - GetVecR()).Length();
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
        Update(PhysicsEditorDefault, InstanceUIEditorDefault);
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