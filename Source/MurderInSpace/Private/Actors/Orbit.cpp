#include "Actors/Orbit.h"

#include <numeric>

#include "Components/SplineMeshComponent.h"
#include "Lib/FunctionLib.h"
#include "Net/UnrealNetwork.h"

AOrbit::AOrbit()
{
	PrimaryActorTick.bCanEverTick = true;
	bNetLoadOnClient = false;
	bReplicates = true;
	AActor::SetReplicateMovement(false);
	
    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    Root->SetMobility(EComponentMobility::Stationary);
    SetRootComponent(Root);
    
    Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Orbit"));
	Spline->SetMobility(EComponentMobility::Stationary);
    Spline->SetupAttachment(Root);

    SplineMeshParent = CreateDefaultSubobject<USceneComponent>(TEXT("SplineMeshes"));
    SplineMeshParent->SetupAttachment(Root);
    SplineMeshParent->SetMobility(EComponentMobility::Stationary);
}

void AOrbit::BeginPlay()
{
    Super::BeginPlay();

    const FInstanceUI InstanceUI = UStateLib::GetInstanceUIUnsafe(this);
    bool bHasProblems = false;

    if(!Body)
    {
        UE_LOG(LogActor, Error, TEXT("%s: no body"), *GetFullName())
        bHasProblems = true;
        SetActorTickEnabled(false);
    }
    
    // Only care for and spline static mesh and material if this orbit is meant to be visible
    if(bTrajectoryShowSpline)
    {
        if(!MSplineMesh)
        {
            UE_LOG(LogActor, Warning, TEXT("%s: spline mesh material not set"), *GetFullName())
            bHasProblems = true;
        }
        if(!StaticMesh)
        {
            UE_LOG(LogActor, Warning, TEXT("%s: static mesh for trajectory not set"), *GetFullName())
            bHasProblems = true;
        }
    }
    if(bHasProblems)
    {
        return;
    }
    
    Update(UStateLib::GetPhysicsUnsafe(this), InstanceUI);
    
    // ignore the visibility set in the editor
    bIsVisibleVarious = false;
    UpdateVisibility(InstanceUI);

}

void AOrbit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if(!RP_bHasBeenSet)
    {
        UE_LOG(LogActor, Error, TEXT("%s: TickComponent: not initialized"), *GetFullName())
        SetActorTickEnabled(false);
        return;
    }
    
    const FPhysics Physics = UStateLib::GetPhysicsUnsafe(this);
    const FVector VecRKepler = GetVecRKepler(Physics);

    Velocity = NextVelocity(VecRKepler.Length(), Physics.Alpha, Velocity, DeltaTime, VecVelocity.Dot(VecRKepler));
    VelocityVCircle = Velocity / GetCircleVelocity(Physics);
    const float DeltaR = Velocity * DeltaTime;

    // advance on spline
    FVector NewVecR;
    SplineDistance = fmod(SplineDistance + DeltaR, Spline->GetSplineLength());
    if(RP_Params.OrbitType == EOrbitType::LINEBOUND)
    {
        VecVelocity = Spline->GetTangentAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World).GetSafeNormal() * Velocity;
        NewVecR = Spline->GetLocationAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World);
    }
    else
    {
        // direction at current position, i.e. at current spline key
        VecVelocity = Spline->GetTangentAtSplineInputKey(SplineKey, ESplineCoordinateSpace::World).GetSafeNormal() * Velocity;
        const FVector NewLocationAtTangent = GetVecR() + VecVelocity * DeltaTime;
        
        // new spline key
        SplineKey = Spline->FindInputKeyClosestToWorldLocation(NewLocationAtTangent);
        NewVecR = Spline->GetLocationAtSplineInputKey(SplineKey, ESplineCoordinateSpace::World);
    }
    GetBody()->SetActorLocation(NewVecR, true, nullptr);
    
    // TODO: account for acceleration
    // const auto RealDeltaR = (GetVecR() - VecR).Length();
    // const auto RelativeError = DeltaR / RealDeltaR - 1.;
    // if(abs(RelativeError) > 0.02)
    // {
    // 	UE_LOG(LogTemp, Warning, TEXT("%s: Expected: %f; really: %f; relative error: %.1f%"), *GetFName().ToString(), DeltaR, RealDeltaR, RelativeError * 100.);
    // }

}

void AOrbit::Update(FPhysics Physics, FInstanceUI InstanceUI)
{
    const FVector VecR = GetBody()->GetActorLocation();
    
    // transform location vector r to Kepler coordinates, where F1 is the origin
    const FVector VecRKepler = GetVecRKepler(Physics);
    const float RKepler = VecRKepler.Length();

    // TODO:
    //const auto VelocityVCircle = Velocity / sqrt(Alpha / R);

    // the bigger this value, the earlier an eccentricity close to 1 will be interpreted as parabola orbit
    constexpr float Tolerance = 1E-2;
    const FVector VecH = VecRKepler.Cross(VecVelocity);
    RP_Params.P = VecH.SquaredLength() / Physics.Alpha;
    Velocity = VecVelocity.Length();
    VelocityVCircle = Velocity / GetCircleVelocity(Physics);
    RP_Params.Energy = pow(Velocity, 2) / 2. - Physics.Alpha / RKepler;
    const FVector VecE = UFunctionLib::Eccentricity(VecRKepler, VecVelocity, Physics.Alpha);
    RP_Params.Eccentricity = VecE.Length();
    const FVector VecENorm = VecE.GetSafeNormal();
    const FVector VecHNorm = VecH.GetSafeNormal();

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
                        , FSplinePoint(1, VecVNorm * Apsis + Physics.VecF1, ESplinePointType::Linear)
                    };
                AddPointsToSpline();
                Spline->SetClosedLoop(false, false);
                Spline->UpdateSpline();
            }
            RP_DistanceZero = Spline->GetDistanceAlongSplineAtSplineInputKey(Spline->FindInputKeyClosestToWorldLocation
                ( VecR)
                );
            SplineDistance = RP_DistanceZero;
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
            RP_Params.A = 0;
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
    else if(RP_Params.Eccentricity <= 1. - Tolerance || (RP_Params.Eccentricity <= 1 && RP_Params.Energy < E_BOUND_MIN))
    {
        RP_Params.A = RP_Params.P / (1 - VecE.SquaredLength());
        const float B = RP_Params.A * sqrt(1 - VecE.SquaredLength());
        const FVector Vertex1 = RP_Params.A * (1 - VecE.Length()) * VecENorm;
        const FVector Vertex2 = RP_Params.A * (1 + VecE.Length()) * -VecENorm;
        const FVector Orthogonal = VecH.Cross(VecE).GetSafeNormal();
        const FVector Covertex1 =  B * Orthogonal - RP_Params.A * VecE;
        const FVector Covertex2 = -B * Orthogonal - RP_Params.A * VecE;
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
    else if(RP_Params.Eccentricity <= 1. + Tolerance)
    {
        std::list<FVector> Points;
        const FVector VecHorizontal = VecHNorm.Cross(VecENorm);
        constexpr int MAX_POINTS = 20;
        const float MAX_N = sqrt(2 * (Physics.WorldRadius + Physics.VecF1.Length()) / RP_Params.P);
        const float Delta = 2 * MAX_N / MAX_POINTS;

        Points.emplace_front(VecENorm * RP_Params.P / (1. + RP_Params.Eccentricity) + Physics.VecF1);
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
        std::list<FVector> Points;
        RP_Params.A = RP_Params.P / (1 - VecE.SquaredLength()); // A < 0
        const float C = RP_Params.P * RP_Params.Eccentricity / (VecE.SquaredLength() - 1);
        const FVector VecHorizontal = VecHNorm.Cross(VecENorm);
        constexpr int MAX_POINTS = 20;
        const float MAX = sqrt((pow(Physics.WorldRadius, 2) + (VecE.SquaredLength() - 1.) * pow(RP_Params.A, 2)) / pow(RP_Params.Eccentricity, 2));
        const float Delta = 2. * MAX / (pow(MAX_POINTS / 2 - 1, 3) / 3.);

        Points.emplace_front(VecENorm * RP_Params.P / (1. + RP_Params.Eccentricity) + Physics.VecF1);
        //Points.emplace_front(VecE * A + VecF1);
        for(int i = 1; i < MAX_POINTS / 2; i++)
        {
            const float X = pow(i, 2) * Delta - RP_Params.A;
            const FVector VecX = (C - X) * VecENorm;
            //(P - sqrt(VecE.SquaredLength() - 1) * RMAX) * VecENorm + VecF1;	
            //const auto VecY = VecHorizontal * sqrt((VecE.SquaredLength() - 1.) * pow(X, 2) - 1.);
            const FVector VecY = VecHorizontal * sqrt((VecE.SquaredLength() - 1.) * (pow(X, 2) - pow(RP_Params.A, 2)));
            Points.emplace_back(VecX + VecY + Physics.VecF1);
            Points.emplace_front(VecX - VecY + Physics.VecF1);
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
        RP_Params.OrbitType = EOrbitType::HYPERBOLA;
        RP_Params.Period = 0;
    }
    Spline->UpdateSpline();

    if(RP_Params.OrbitType != EOrbitType::LINEBOUND)
    {
        SplineKey = Spline->FindInputKeyClosestToWorldLocation(VecR);
        RP_DistanceZero = Spline->GetDistanceAlongSplineAtSplineInputKey(SplineKey);
        SplineDistance = RP_DistanceZero;
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
        SpawnSplineMesh
            ( SplineMeshColor
            , SplineMeshParent
            , InstanceUI
            );
    }
}

void AOrbit::UpdateSplineMeshScale(float InScaleFactor)
{
    SplineMeshScaleFactor = InScaleFactor;
    TArray<USceneComponent*> Children;
    SplineMeshParent->GetChildrenComponents(false, Children);
    for(const auto Child : Children)
    {
        const auto Mesh = Cast<USplineMeshComponent>(Child);
        const auto Material = Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(0));
        Material->SetScalarParameterValue(FName(TEXT("NumBands")), 4. / InScaleFactor);
        
        Mesh->SetStartScale(SplineMeshScaleFactor * FVector2D::UnitVector, false);
        Mesh->SetEndScale(SplineMeshScaleFactor * FVector2D::UnitVector, false);
        Mesh->UpdateMesh();
    }
}

float AOrbit::VelocityEllipse(float R, float Alpha)
{
    return std::max(sqrt(Alpha * (2.0 / R - 1.0 / RP_Params.A)), 1.);
}

float AOrbit::VelocityParabola(float R, float Alpha)
{
    return sqrt(Alpha * 2.0 / R);
}

float AOrbit::NextVelocity(float R, float Alpha, float OldVelocity, float DeltaTime, float Sign)
{
    if(!RP_bHasBeenSet)
    {
        UE_LOG(LogActor, Error, TEXT("%s: NextVelocity: not initialized"), *GetFullName());
        return 0.;
    }
    
    switch(RP_Params.OrbitType)
    {
    case EOrbitType::CIRCLE:
        return OldVelocity;
    case EOrbitType::ELLIPSE:
        return VelocityEllipse(R, Alpha);
    case EOrbitType::LINEBOUND:
        return VelocityEllipse(R, Alpha);
    case EOrbitType::LINEUNBOUND:
        return OldVelocity - copysign(Alpha / pow(R, 2) * DeltaTime, Sign);
    case EOrbitType::PARABOLA:
        return VelocityParabola(R, Alpha);
    case EOrbitType::HYPERBOLA:
        return VelocityEllipse(R, Alpha);
    default:
        UE_LOG(LogTemp, Error, TEXT("AOrbit::NextVelocity: Impossible"))
        return 0;
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

bool AOrbit::GetVisibility(FInstanceUI InstanceUI) const
{
    return bIsVisibleVarious
        || bIsVisibleAccelerating
        || InstanceUI.bShowAllTrajectories
        || (InstanceUI.Selected && InstanceUI.Selected->Orbit == this);
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
    return StrOrbitType + FString::Printf(TEXT(", E = %.2f, P = %.1f, Energy = %.1f, Period = %.1f, A = %.1f"), RP_Params.Eccentricity, RP_Params.P, RP_Params.Energy, RP_Params.Period, RP_Params.A);
}

float AOrbit::GetCircleVelocity(FPhysics Physics) const
{
    return sqrt(Physics.Alpha / GetVecRKepler(Physics).Length());
}

void AOrbit::AddVelocity(FVector VecDeltaV, FPhysics Physics, FInstanceUI InstanceUI)
{
    SetVelocity(VecVelocity + VecDeltaV);
    Update(Physics, InstanceUI);
}

void AOrbit::OnRep_OrbitState()
{
    // OrbitState is replicated with condition "initial only", implying that replication (including the call
    // to this method) happens only once
    GetBody()->SetActorLocation(RP_OrbitState.VecR, true, nullptr);
    VecVelocity    = RP_OrbitState.VecVelocity;
    Velocity       = VecVelocity.Length();
    SplineKey      = Spline->FindInputKeyClosestToWorldLocation(RP_OrbitState.VecR);
    SplineDistance = Spline->GetDistanceAlongSplineAtSplineInputKey(SplineKey);
}

void AOrbit::SetCircleOrbit(FVector InVecR, FPhysics Physics)
{
    const FVector VecRKepler = InVecR - Physics.VecF1;
    const FVector VelocityNormal = FVector(0., 0., 1.).Cross(VecRKepler).GetSafeNormal(1e-8, FVector(0., 1., 0.));
    const FVector NewVecVelocity = VelocityNormal * sqrt(Physics.Alpha / VecRKepler.Length());
    SetOrbitByParams(InVecR, NewVecVelocity, Physics);
}

void AOrbit::SetOrbitByParams(FVector InVecR, FVector InVecVelocity, FPhysics Physics)
{
    GetBody()->SetActorLocation(InVecR, true, nullptr);
    SetVelocity(InVecVelocity);
    RP_bHasBeenSet = true;
}

void AOrbit::UpdateVisibility(FInstanceUI InstanceUI)
{
    Spline->SetVisibility(GetVisibility(InstanceUI), true);
}

void AOrbit::SpawnSplineMesh(FLinearColor Color, USceneComponent* InParent, FInstanceUI InstanceUI)
{
    const int nIndices = static_cast<int>(round(Spline->GetSplineLength() / SplineMeshLength));

    if(GetOwner()->HasAnyFlags(RF_ClassDefaultObject))
    {
        return;
    }
    if(!StaticMesh)
    {
        UE_LOG
        ( LogActor
        , Warning
        , TEXT("%s: AOrbit::SpawnSplineMesh: SMSplineMesh null, skipping")
        , *GetFullName()
        )
        return;
    }
    if(!MSplineMesh)
    {
        UE_LOG
        ( LogActor
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
            
            USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(InParent);
            
            SplineMesh->SetMobility(EComponentMobility::Stationary);
            SplineMesh->SetVisibility(GetVisibility(InstanceUI));
            
            // if I dont register here, the spline mesh doesn't render
            SplineMesh->RegisterComponent();
            // in theory, I could use `SetupAttachment` and then `RegisterComponent`
            // in practice, that messes up the location (i.e. it moves the spline mesh)
            // and the spline meshes do not show up with their correct names in the editor
            SplineMesh->AttachToComponent(InParent, FAttachmentTransformRules::KeepWorldTransform);
            // if I don't add instance here, the spline meshes don't show in the component list in the editor
            GetOwner()->AddInstanceComponent(SplineMesh);

            SplineMesh->CastShadow = false;
            SplineMesh->SetStaticMesh(StaticMesh);

            UMaterialInstanceDynamic* DynamicMaterial =
                SplineMesh->CreateDynamicMaterialInstance(0, MSplineMesh);
            DynamicMaterial->SetVectorParameterValue(FName(TEXT("StripesColor")), Color);

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
    
    const FPhysics Physics = UStateLib::GetPhysicsEditorDefault();
    const FInstanceUI InstanceUI = UStateLib::GetInstanceUIEditorDefault();
    
    const FName Name = PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName();

    static const FName FNameBTrajectoryShowSpline = GET_MEMBER_NAME_CHECKED(AOrbit, bTrajectoryShowSpline);
    static const FName FNameBVisibleVarious       = GET_MEMBER_NAME_CHECKED(AOrbit, bIsVisibleVarious    );
    static const FName FNameSplineMeshLength      = GET_MEMBER_NAME_CHECKED(AOrbit, SplineMeshLength     );
    static const FName FNameVelocity              = GET_MEMBER_NAME_CHECKED(AOrbit, Velocity             );
    static const FName FNameVelocityVCircle       = GET_MEMBER_NAME_CHECKED(AOrbit, VelocityVCircle      );
    static const FName FNameVecVelocity           = GET_MEMBER_NAME_CHECKED(AOrbit, VecVelocity          );
    static const FName FNameSMTrajectory          = GET_MEMBER_NAME_CHECKED(AOrbit, StaticMesh         );
    static const FName FNameSplineMeshMaterial    = GET_MEMBER_NAME_CHECKED(AOrbit, MSplineMesh          );
    static const FName FNameSplineMeshColor       = GET_MEMBER_NAME_CHECKED(AOrbit, SplineMeshColor      );
    static const FName FNameSplineMeshScaleFactor = GET_MEMBER_NAME_CHECKED(AOrbit, SplineMeshScaleFactor);

    if(Name == FNameBVisibleVarious)
    {
        UpdateVisibility(InstanceUI);
    }
    else if
        (  Name == FNameSplineMeshLength
        || Name == FNameBTrajectoryShowSpline
        || Name == FNameSMTrajectory
        || Name == FNameSplineMeshMaterial
        || Name == FNameSplineMeshColor
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
            VecVelocity = Velocity * VelocityNormal;
        }
        else if(Name == FNameVelocityVCircle)
        {
            VecVelocity = GetCircleVelocity(Physics) * VelocityVCircle * VelocityNormal;
        }
        else if(Name == FNameVecVelocity)
        {
        }
    }
    Update(Physics, InstanceUI);
}
#endif

void AOrbit::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(AOrbit, RP_bClosedLoop )

    DOREPLIFETIME(AOrbit, RP_Params      )
    DOREPLIFETIME(AOrbit, RP_DistanceZero)
    DOREPLIFETIME(AOrbit, RP_SplinePoints)
    
    DOREPLIFETIME_CONDITION(AOrbit, RP_OrbitState, COND_InitialOnly )
}

void AOrbit::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
    Super::PreReplication(ChangedPropertyTracker);

    DOREPLIFETIME_ACTIVE_OVERRIDE(AOrbit, RP_Params      , !bIsChanging)
    DOREPLIFETIME_ACTIVE_OVERRIDE(AOrbit, RP_DistanceZero, !bIsChanging)
    DOREPLIFETIME_ACTIVE_OVERRIDE(AOrbit, RP_SplinePoints, !bIsChanging)
}
