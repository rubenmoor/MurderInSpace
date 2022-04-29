// Fill out your copyright notice in the Description page of Project Settings.


#include "KeplerOrbitComponent.h"

#include "FunctionLib.h"
#include "MyGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UKeplerOrbitComponent::Initialize(FVector _VecF1, TObjectPtr<USceneComponent> _Body, TObjectPtr<UHierarchicalInstancedStaticMeshComponent> _TrajectoryMesh)
{
	VecF1 = _VecF1;
	Body = _Body;
	TrajectoryMesh = _TrajectoryMesh;
	SetAbsolute(true, true, true);
	TrajectoryMesh->SetAbsolute(true, true, true);
	bHasOrbit = false;
	bInitialized = true;
}

void UKeplerOrbitComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	const auto VecR = Body->GetComponentLocation();
	const auto VecRKepler = VecR - VecF1;
	const auto R = VecRKepler.Length();
	const auto MU = GetOwner()->GetGameInstance<UMyGameInstance>()->MU;
	switch(Orbit)
	{
	case orbit::CIRCLE:
		break;
	case orbit::ELLIPSE:
	 	Velocity = UFunctionLib::VelocityEllipse(R, A, MU);
		break;
	case orbit::LINEBOUND:
		Velocity = UFunctionLib::VelocityEllipse(R, A, MU);
		break;
	case orbit::LINEUNBOUND:
		Velocity = Velocity - copysign(MU / pow(R, 2) * DeltaTime, VecVelocity.Dot(VecRKepler));
		break;
	case orbit::PARABOLA:
	 	Velocity = UFunctionLib::VelocityParabola(R, MU);
		break;
	case orbit::HYPERBOLA:
	 	Velocity = UFunctionLib::VelocityEllipse(R, A, MU);
		break;
	}
	VelocityNormalized = Velocity / sqrt(MU / R);
	const auto DeltaR = Velocity * DeltaTime;

	if(Orbit == orbit::LINEBOUND)
	{
		SplineDistanceLineBound = fmod(SplineDistanceLineBound + DeltaR, GetSplineLength());
		Body->SetWorldLocation(GetLocationAtDistanceAlongSpline(SplineDistanceLineBound, ESplineCoordinateSpace::World));
		VecVelocity = GetTangentAtDistanceAlongSpline(SplineDistanceLineBound, ESplineCoordinateSpace::World).GetSafeNormal() * Velocity;
	}
	else
	{
		const auto Key = FindInputKeyClosestToWorldLocation(VecR);
		VecVelocity = GetTangentAtSplineInputKey(Key, ESplineCoordinateSpace::World).GetSafeNormal() * Velocity;
		const auto NewLocation = VecR + VecVelocity * DeltaTime;
		Body->SetWorldLocation(FindLocationClosestToWorldLocation(NewLocation, ESplineCoordinateSpace::World));
	}
	
	const auto RealDeltaR = (Body->GetComponentLocation() - VecR).Length();
	const auto RelativeError = DeltaR / RealDeltaR - 1.;
	if(abs(RelativeError) > 0.02)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Expected: %f; really: %f; relative error: %.1f%"), *GetOwner()->GetFName().ToString(), DeltaR, RealDeltaR, RelativeError * 100.);
	}
}

void UKeplerOrbitComponent::UpdateOrbit(FVector VecR, FVector VecV, float MU, float RMAX)
{
	if(!bInitialized)
	{
		RequestEngineExit(TEXT("KeplerOrbitComponent: need to run Initialize."));
	}

	SetWorldLocation(VecF1);
	TrajectoryMesh->SetWorldLocation(VecF1);
	VecVelocity = VecV;
	Velocity = VecV.Length();
	
	ClearSplinePoints(false);

	// transform location vector r to Kepler coordinates, where F1 is the origin
	const auto VecRKepler = VecR - VecF1;
	const auto R = VecRKepler.Length();
	VelocityNormalized = Velocity / sqrt(MU / R);

	// the bigger this value, the earlier an eccentricity close to 1 will be interpreted as parabola orbit
	constexpr auto Tolerance = 1E-2;
	const auto VecH = VecRKepler.Cross(VecV);
	P = VecH.SquaredLength() / MU;
	Energy = pow(Velocity, 2) / 2. - MU / R;
	const auto VecE = UFunctionLib::Eccentricity(VecRKepler, VecV, MU);
	Eccentricity = VecE.Length();
	const auto VecENorm = VecE.GetSafeNormal();
	const auto VecHNorm = VecH.GetSafeNormal();

	// the energy of the weakest bound state: a circular orbit at R_MAX
	const auto E_BOUND_MIN = -MU / (2 * RMAX);

	// H = 0 (implies E = 1, too): falling in a straight line
	if(VecHNorm.IsZero())
	{
		const auto EMIN = -MU / RMAX;
		const auto E = VecV.SquaredLength() / 2. - MU / VecRKepler.Length();

		// bound
		if(E < EMIN)
		{
			const auto Apsis = -MU / E;
			const auto VecVNorm = VecV.GetSafeNormal();
			if(VecVNorm.IsZero())
			{
				AddPoints(
					{ FSplinePoint(0, VecR, ESplinePointType::Linear)
					, FSplinePoint(1, -VecRKepler + VecF1, ESplinePointType::Linear)
					});
			}
			else
			{
				AddPoints(
					{ FSplinePoint(0, -VecVNorm * Apsis + VecF1, ESplinePointType::Linear)
						, FSplinePoint(1, VecVNorm * Apsis + VecF1, ESplinePointType::Linear)
					});
				SetClosedLoop(false, false);
				UpdateSpline();
			}
			SetClosedLoop(true, false);
			Orbit = orbit::LINEBOUND;
			SplineDistanceLineBound = GetDistanceAlongSplineAtSplineInputKey(FindInputKeyClosestToWorldLocation(VecR));
			A = UFunctionLib::SemiMajorAxis(VecRKepler, VecV, MU);
			Period = 0; // TODO
		}

		// unbound
		else
		{
			AddPoints(
				{ FSplinePoint(0, VecR, ESplinePointType::Linear)
					, FSplinePoint(1, VecV.GetUnsafeNormal() * (RMAX - VecR.Length()), ESplinePointType::Linear)
				});
			SetClosedLoop(false, false);
			Orbit = orbit::LINEUNBOUND;
			A = 0;
			Period = 0;
		}
	}
	
	// E = 0, circle
	else if(VecENorm.IsZero())
	{
	    const auto VecP2 = VecHNorm.Cross(VecRKepler);
	    const auto VecT1 = VecHNorm.Cross(VecRKepler) * SplineToCircle;
	    const auto VecT4 = VecRKepler * SplineToCircle;
		AddPoints(
	      { FSplinePoint(0,  VecR ,  VecT1,  VecT1)
		    , FSplinePoint(1, VecP2 + VecF1, -VecT4, -VecT4)
		    , FSplinePoint(2, -VecRKepler + VecF1 , -VecT1, -VecT1)
		    , FSplinePoint(3, -VecP2 + VecF1,  VecT4,  VecT4)
		    });
    	SetClosedLoop(true, false);
		Orbit = orbit::CIRCLE;
		A = R;
		Period = UFunctionLib::PeriodEllipse(R, MU);
	}

	// 0 < E < 1, Ellipse
	// the Tolerance causes a peculiarity:
	// E close to 1 can imply a parabola, but only for Energy approaching 0 (and P > 0);
	// E close to 1 within the given `Tolerance`, allows for Energy * P / MU in the range
	// [ Tolerance^2 / 2 - Tolerance, Tolerance^2 + Tolerance], which is a huge range for Energy, given P,
	// and any bound orbit that should be just a narrow ellipse, jumps to being a wrong parabola;
	// those "narrow ellipses" imply small P values; P / (1 + Eccentricity) equals the periapsis; thus:
	// small values for the periapsis that are impossible Kepler orbits because of collision with the main body.
	// For a black hole, a P cutoff isn't ideal (small Periapses are allowed), thus instead I define a cutoff energy:
	// The total energy at R_MAX when orbiting in a circle is the bound state with energy closest to zero.
	// As long the energy is smaller than that, we can safely assume a bound state and thus an ellipse instead of
	// the parabola
	else if(Eccentricity <= 1. - Tolerance || (Eccentricity <= 1 && Energy < E_BOUND_MIN))
	{
		A = P / (1 - VecE.SquaredLength());
	    const auto B = A * sqrt(1 - VecE.SquaredLength());
	    const auto Vertex1 = A * (1 - VecE.Length()) * VecENorm;
	    const auto Vertex2 = A * (1 + VecE.Length()) * -VecENorm;
	    const auto Orthogonal = VecH.Cross(VecE).GetSafeNormal();
	    const auto Covertex1 =  B * Orthogonal - A * VecE;
	    const auto Covertex2 = -B * Orthogonal - A * VecE;
	    const auto T1 = Orthogonal * SplineToCircle * B;
	    const auto T4 = VecENorm * SplineToCircle * A;
		AddPoints(
	      { FSplinePoint(0, Vertex1   + VecF1,  T1,  T1)
		    , FSplinePoint(1, Covertex1 + VecF1, -T4, -T4)
		    , FSplinePoint(2, Vertex2   + VecF1, -T1, -T1)
		    , FSplinePoint(3, Covertex2 + VecF1,  T4,  T4)
		    });
    	SetClosedLoop(true, false);
		Orbit = orbit::ELLIPSE;
		Period = UFunctionLib::PeriodEllipse(A, MU);
	}
	
	// E = 1, Parabola
	else if(Eccentricity <= 1. + Tolerance)
	{
		std::list<FVector> Points;
	    const auto VecHorizontal = VecHNorm.Cross(VecENorm);
		constexpr auto MAX_POINTS = 20;
		const auto MAX_N = sqrt(2 * (RMAX + VecF1.Length()) / P);
		const auto Delta = 2 * MAX_N / MAX_POINTS;
		
		Points.emplace_front(VecENorm * P / (1. + Eccentricity) + VecF1);
		for(int i = 1; i < MAX_POINTS / 2; i++)
		{
			const auto VecX = i * Delta * VecHorizontal * P;
			const auto VecY = VecENorm / 2. * (1 - pow(i * Delta, 2)) * P;
			Points.emplace_back(VecY + VecX + VecF1);
			Points.emplace_front(VecY - VecX + VecF1);
		}
		
		for(auto Point : Points)
		{
			AddSplineWorldPoint(Point);
		}
    	SetClosedLoop(false, false);
		Orbit = orbit::PARABOLA;
		Period = 0;
		A = 0;
	}

	// E > 1, Hyperbola
	else
	{
		std::list<FVector> Points;
		A = P / (1 - VecE.SquaredLength()); // A < 0
		const auto C = P * Eccentricity / (VecE.SquaredLength() - 1);
	    const auto VecHorizontal = VecHNorm.Cross(VecENorm);
		constexpr auto MAX_POINTS = 20;
		const auto MAX = sqrt((pow(RMAX, 2) + (VecE.SquaredLength() - 1.) * pow(A, 2)) / pow(Eccentricity, 2));
		const auto Delta = 2. * MAX / (pow(MAX_POINTS / 2 - 1, 3) / 3.);

		Points.emplace_front(VecENorm * P / (1. + Eccentricity) + VecF1);
		//Points.emplace_front(VecE * A + VecF1);
		for(int i = 1; i < MAX_POINTS / 2; i++)
		{
			const auto X = pow(i, 2) * Delta - A;
			const auto VecX = (C - X) * VecENorm;
			//(P - sqrt(VecE.SquaredLength() - 1) * RMAX) * VecENorm + VecF1;	
			//const auto VecY = VecHorizontal * sqrt((VecE.SquaredLength() - 1.) * pow(X, 2) - 1.);
			const auto VecY = VecHorizontal * sqrt((VecE.SquaredLength() - 1.) * (pow(X, 2) - pow(A, 2)));
			Points.emplace_back(VecX + VecY + VecF1);
			Points.emplace_front(VecX - VecY + VecF1);
		}
			
		for(auto Point : Points)
		{
			AddSplineWorldPoint(Point);
		}
    	SetClosedLoop(false, false);
		Orbit = orbit::HYPERBOLA;
		Period = 0;
	}
	UpdateSpline();

	const auto SplineDistance = GetDistanceAlongSplineAtSplineInputKey(FindInputKeyClosestToWorldLocation(VecR));
	const auto MAX_DISTANCE = std::min<float>(10000., GetSplineLength());
	const auto DISTANCE_DELTA = 50.f;
	const auto N_MAX = MAX_DISTANCE / DISTANCE_DELTA;
	TrajectoryMesh->ClearInstances();
	for(int i = 1; i < N_MAX; i++)
	{
		const auto Transform = GetTransformAtDistanceAlongSpline(SplineDistance + i * DISTANCE_DELTA, ESplineCoordinateSpace::World);
		TrajectoryMesh->AddInstance(Transform);
	}
	bHasOrbit = true;
}

/**
 * @brief update orbit maintaining the characteristics as best as possible
 * @param MU gravitational parameter
 * @param RMAX maximum distance where stuff is simulated
 */
void UKeplerOrbitComponent::UpdateOrbit(const float MU, const float RMAX)
{
	const auto VecR = Body->GetComponentLocation();
	UpdateOrbit(VecR, VecVelocity, MU, RMAX);
}

void UKeplerOrbitComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedChainEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedChainEvent);
	const auto Name = PropertyChangedChainEvent.PropertyChain.GetHead()->GetValue()->GetFName();
	static const FName FNameVelocityScalar = GET_MEMBER_NAME_CHECKED(UKeplerOrbitComponent, Velocity);
	static const FName FNameVelocityNormalized = GET_MEMBER_NAME_CHECKED(UKeplerOrbitComponent, VelocityNormalized);
	static const FName FNameVelocity = GET_MEMBER_NAME_CHECKED(UKeplerOrbitComponent, VecVelocity);
	static const FName FNameEccentricity = GET_MEMBER_NAME_CHECKED(UKeplerOrbitComponent, Eccentricity);
	
	const auto VecR = Body->GetComponentLocation();
	constexpr auto MU = DefaultMU;
	const auto RMAX = UMyGameInstance::DefaultGameAreaRadius;
	const auto VecRKepler = VecR - VecF1;
	FVector NewVelocity;
	
	const auto VelocityNormal = VecVelocity.GetSafeNormal(1e-8, FVector(0., 1., 0.));
	
	if(Name == FNameVelocityScalar)
	{
		NewVelocity = Velocity * VelocityNormal;
		UpdateOrbit(VecR, NewVelocity, MU, RMAX);
	}
	else if(Name == FNameVelocityNormalized)
	{
		NewVelocity = sqrt(MU / VecRKepler.Length()) * VelocityNormalized * VelocityNormal;
		UpdateOrbit(VecR, NewVelocity, MU, RMAX);
	}
	else if(Name == FNameVelocity)
	{
		NewVelocity = VecVelocity;
		UpdateOrbit(VecR, NewVelocity, MU, RMAX);
	}
	else if(Name == FNameEccentricity)
	{
		if(Eccentricity < 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Eccentricity can't be negative"));
			Eccentricity = 0;
		}
		UE_LOG(LogTemp, Warning, TEXT("In UKeplerOrbitComponent::PostEditChangeProperty: Eccentricty: not implemented"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("In UKeplerOrbitComponent::PostEditChangeProperty: %s, not doing anything"), *Name.ToString());
	}
}

void UKeplerOrbitComponent::BeginPlay()
{
	Super::BeginPlay();
	if(!bHasOrbit)
	{
		// set up default circular orbit
		const auto VecR = Body->GetComponentLocation();
		const auto GI = GetOwner()->GetGameInstance<UMyGameInstance>();
		const auto VecV = FVector(0, 0, 1).Cross(VecR).GetSafeNormal() * sqrt(GI->MU / VecR.Length());
		UpdateOrbit(VecR, VecV, GI->MU, GI->GameAreaRadius);
	}
}
