#include "Orbit.h"

#include <numeric>

#include "FunctionLib.h"
#include "MyGameInstance.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/SplineMeshComponent.h"

AOrbit::AOrbit()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	Spline->SetupAttachment(Root);
	Spline->ClearSplinePoints();

	HISMTrajectory = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("HISMTrajectory"));
	HISMTrajectory->SetupAttachment(Root);
}

void AOrbit::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedChainEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedChainEvent);

	if(!ActorInSpace)
	{
		UE_LOG(LogTemp, Warning, TEXT("AOrbit::PostEditChangeChainProperty: ActorInSpace null, ignoring"));
	}
	else
	{
		const auto Name = PropertyChangedChainEvent.PropertyChain.GetHead()->GetValue()->GetFName();
		static const FName FNameVelocityScalar = GET_MEMBER_NAME_CHECKED(AOrbit, Velocity);
		static const FName FNameVelocityNormalized = GET_MEMBER_NAME_CHECKED(AOrbit, VelocityNormalized);
		static const FName FNameVelocity = GET_MEMBER_NAME_CHECKED(AOrbit, VecVelocity);
		static const FName FNameEccentricity = GET_MEMBER_NAME_CHECKED(AOrbit, Eccentricity);
		
		const auto VecR = ActorInSpace->GetActorLocation();
		constexpr auto ALPHA = UMyGameInstance::EditorDefaultAlpha;
		const auto RMAX = UMyGameInstance::EditorDefaultWorldRadiusUU;
		const auto VecRKepler = VecR - VecF1;
		FVector NewVelocity;
		
		const auto VelocityNormal = VecVelocity.GetSafeNormal(1e-8, FVector(0., 1., 0.));

		if(Name == FNameVelocityScalar)
		{
			NewVelocity = Velocity * VelocityNormal;
			UpdateOrbit(NewVelocity, ALPHA, RMAX);
		}
		else if(Name == FNameVelocityNormalized)
		{
			NewVelocity = sqrt(ALPHA / VecRKepler.Length()) * VelocityNormalized * VelocityNormal;
			UpdateOrbit(NewVelocity, ALPHA, RMAX);
		}
		else if(Name == FNameVelocity)
		{
			NewVelocity = VecVelocity;
			UpdateOrbit(NewVelocity, ALPHA, RMAX);
		}
		else if(Name == FNameEccentricity)
		{
			if(Eccentricity < 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("Eccentricity can't be negative"));
				Eccentricity = 0;
			}
			UE_LOG(LogTemp, Warning, TEXT("In UKeplerOrbitComponent::PostEditChangeProperty: Eccentricity: not implemented"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("In UKeplerOrbitComponent::PostEditChangeProperty: %s, not doing anything"), *Name.ToString());
		}
	}
}

void AOrbit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const auto VecR = ActorInSpace->GetActorLocation();
	const auto VecRKepler = VecR - VecF1;
	const auto R = VecRKepler.Length();
	const auto Alpha = GetGameInstance<UMyGameInstance>()->Alpha;
	switch(Orbit)
	{
	case OrbitType::CIRCLE:
		break;
	case OrbitType::ELLIPSE:
	 	Velocity = UFunctionLib::VelocityEllipse(R, A, Alpha);
		break;
	case OrbitType::LINEBOUND:
		Velocity = UFunctionLib::VelocityEllipse(R, A, Alpha);
		break;
	case OrbitType::LINEUNBOUND:
		Velocity = Velocity - copysign(Alpha / pow(R, 2) * DeltaTime, VecVelocity.Dot(VecRKepler));
		break;
	case OrbitType::PARABOLA:
	 	Velocity = UFunctionLib::VelocityParabola(R, Alpha);
		break;
	case OrbitType::HYPERBOLA:
	 	Velocity = UFunctionLib::VelocityEllipse(R, A, Alpha);
		break;
	}
	VelocityNormalized = Velocity / sqrt(Alpha / R);
	const auto DeltaR = Velocity * DeltaTime;

	SplineDistance = fmod(SplineDistance + DeltaR, Spline->GetSplineLength());
	if(Orbit == OrbitType::LINEBOUND)
	{
		ActorInSpace->SetActorLocation(Spline->GetLocationAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World));
		VecVelocity = Spline->GetTangentAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World).GetSafeNormal() * Velocity;
	}
	else
	{
		// direction at current position, i.e. at current spline key
		VecVelocity = Spline->GetTangentAtSplineInputKey(SplineKey, ESplineCoordinateSpace::World).GetSafeNormal() * Velocity;
		const auto NewLocation = VecR + VecVelocity * DeltaTime;
		
		// new spline key
		SplineKey = Spline->FindInputKeyClosestToWorldLocation(NewLocation);
		ActorInSpace->SetActorLocation(Spline->GetLocationAtSplineInputKey(SplineKey, ESplineCoordinateSpace::World));

		// update trajectory HISM markers
		if((SplineDistance - DistanceZero) / HISMDistance > HISMCurrentIndex)
		{
			const auto Distance = fmod(DistanceZero + (HISMNumberOfMarkers + HISMCurrentIndex) * HISMDistance, Spline->GetSplineLength());
			const auto Transform = Spline->GetTransformAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
			HISMTrajectory->UpdateInstanceTransform(HISMCurrentIndex % HISMNumberOfMarkers, Transform, true);
			HISMCurrentIndex = (HISMCurrentIndex + 1) % static_cast<int>(std::round(Spline->GetSplineLength() / HISMDistance));
		}
	}
	
	const auto RealDeltaR = (ActorInSpace->GetActorLocation() - VecR).Length();
	const auto RelativeError = DeltaR / RealDeltaR - 1.;
	if(abs(RelativeError) > 0.02)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Expected: %f; really: %f; relative error: %.1f%"), *GetFName().ToString(), DeltaR, RealDeltaR, RelativeError * 100.);
	}
}

void AOrbit::UpdateOrbit(FVector VecV, float Alpha, float RMAX)
{
	if(!ActorInSpace)
	{
		RequestEngineExit(TEXT("AOrbit::UpdateOrbit: ActorInSpace null"));
	}
	
	VecVelocity = VecV;
	Velocity = VecV.Length();
	
	Spline->ClearSplinePoints(false);

	// transform location vector r to Kepler coordinates, where F1 is the origin
	const auto VecR = ActorInSpace->GetActorLocation();
	const auto VecRKepler = VecR - VecF1;
	const auto R = VecRKepler.Length();
	VelocityNormalized = Velocity / sqrt(Alpha / R);

	// the bigger this value, the earlier an eccentricity close to 1 will be interpreted as parabola orbit
	constexpr auto Tolerance = 1E-2;
	const auto VecH = VecRKepler.Cross(VecV);
	P = VecH.SquaredLength() / Alpha;
	Energy = pow(Velocity, 2) / 2. - Alpha / R;
	const auto VecE = UFunctionLib::Eccentricity(VecRKepler, VecV, Alpha);
	Eccentricity = VecE.Length();
	const auto VecENorm = VecE.GetSafeNormal();
	const auto VecHNorm = VecH.GetSafeNormal();

	// the energy of the weakest bound state: a circular orbit at R_MAX
	const auto E_BOUND_MIN = -Alpha / (2 * RMAX);

	// H = 0 (implies E = 1, too): falling in a straight line
	if(VecHNorm.IsZero())
	{
		const auto EMIN = -Alpha / RMAX;
		const auto E = VecV.SquaredLength() / 2. - Alpha / VecRKepler.Length();

		// bound
		if(E < EMIN)
		{
			const auto Apsis = -Alpha / E;
			const auto VecVNorm = VecV.GetSafeNormal();
			if(VecVNorm.IsZero())
			{
				Spline->AddPoints(
					{ FSplinePoint(0, VecR, ESplinePointType::Linear)
					, FSplinePoint(1, -VecRKepler + VecF1, ESplinePointType::Linear)
					});
			}
			else
			{
				Spline->AddPoints(
					{ FSplinePoint(0, -VecVNorm * Apsis + VecF1, ESplinePointType::Linear)
						, FSplinePoint(1, VecVNorm * Apsis + VecF1, ESplinePointType::Linear)
					});
				Spline->SetClosedLoop(false, false);
				Spline->UpdateSpline();
			}
			DistanceZero = Spline->GetDistanceAlongSplineAtSplineInputKey(Spline->FindInputKeyClosestToWorldLocation(VecR));
			SplineDistance = DistanceZero;
			Spline->SetClosedLoop(true, false);
			
			Orbit = OrbitType::LINEBOUND;
			A = UFunctionLib::SemiMajorAxis(VecRKepler, VecV, Alpha);
			Period = UFunctionLib::PeriodEllipse(A, Alpha);
		}

		// unbound
		else
		{
			Spline->AddPoints(
				{ FSplinePoint(0, VecR, ESplinePointType::Linear)
					, FSplinePoint(1, VecV.GetUnsafeNormal() * (RMAX - VecR.Length()), ESplinePointType::Linear)
				});
			Spline->SetClosedLoop(false, false);
			Orbit = OrbitType::LINEUNBOUND;
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
		Spline->AddPoints(
	      { FSplinePoint(0,  VecR ,  VecT1,  VecT1)
		    , FSplinePoint(1, VecP2 + VecF1, -VecT4, -VecT4)
		    , FSplinePoint(2, -VecRKepler + VecF1 , -VecT1, -VecT1)
		    , FSplinePoint(3, -VecP2 + VecF1,  VecT4,  VecT4)
		    });
    	Spline->SetClosedLoop(true, false);
		Orbit = OrbitType::CIRCLE;
		A = R;
		Period = UFunctionLib::PeriodEllipse(R, Alpha);
	}

	// 0 < E < 1, Ellipse
	// the Tolerance causes a peculiarity:
	// E close to 1 can imply a parabola, but only for Energy approaching 0 (and P > 0);
	// E close to 1 within the given `Tolerance`, allows for Energy * P / ALPHA in the range
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
		Spline->AddPoints(
	      { FSplinePoint(0, Vertex1   + VecF1,  T1,  T1)
		    , FSplinePoint(1, Covertex1 + VecF1, -T4, -T4)
		    , FSplinePoint(2, Vertex2   + VecF1, -T1, -T1)
		    , FSplinePoint(3, Covertex2 + VecF1,  T4,  T4)
		    });
    	Spline->SetClosedLoop(true, false);
		Orbit = OrbitType::ELLIPSE;
		Period = UFunctionLib::PeriodEllipse(A, Alpha);
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
			Spline->AddSplineWorldPoint(Point);
		}
    	Spline->SetClosedLoop(false, false);
		Orbit = OrbitType::PARABOLA;
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
			Spline->AddSplineWorldPoint(Point);
		}
    	Spline->SetClosedLoop(false, false);
		Orbit = OrbitType::HYPERBOLA;
		Period = 0;
	}
	Spline->UpdateSpline();

	if(Orbit != OrbitType::LINEBOUND)
	{
		SplineKey = Spline->FindInputKeyClosestToWorldLocation(VecR);
		DistanceZero = Spline->GetDistanceAlongSplineAtSplineInputKey(SplineKey);
		SplineDistance = DistanceZero;
	}
	// else
	// `SplineKey` is not needed,
	// `SplineDistance` and `DistanceZero` are set already
		
	const auto HISMLength = std::min<float>(HISMMaxLength, Spline->GetSplineLength());
	HISMNumberOfMarkers = std::max<int>(1, HISMLength / HISMDistance);

	HISMTrajectory->ClearInstances();
	for(int i = 1; i < HISMNumberOfMarkers; i++)
	{
		const auto Transform = Spline->GetTransformAtDistanceAlongSpline(DistanceZero + i * HISMDistance, ESplineCoordinateSpace::World);
		HISMTrajectory->AddInstance(Transform);
	}
	HISMCurrentIndex = 0;

	TArray<USplineMeshComponent*> OldSplinesMeshes;
	GetComponents(OldSplinesMeshes);
	for(auto Old : OldSplinesMeshes)
	{
		Old->DestroyComponent();
	}

	std::vector<int> Indices(Spline->GetNumberOfSplinePoints());
	std::iota(Indices.begin(), Indices.end(), 0);

	if(Spline->IsClosedLoop() && Orbit != OrbitType::LINEBOUND)
	{
		Indices.push_back(0);
	}
	for(int i = 0; i < Indices.size() - 1; i++)
	{
		const auto SplineMesh = NewObject<USplineMeshComponent>(this, *FString(TEXT("SplineMesh")).Append(FString::FromInt(i)));
		SplineMesh->SetupAttachment(Root);
		SplineMesh->RegisterComponent();
		AddInstanceComponent(SplineMesh);
		SplineMesh->SetMobility(EComponentMobility::Static);
		SplineMesh->CastShadow = false;
		SplineMesh->SetStaticMesh(SM_Trajectory);
		const auto VecStartPos = Spline->GetLocationAtSplinePoint(Indices[i], ESplineCoordinateSpace::World);
		const auto VecStartDirection = Spline->GetTangentAtSplinePoint(Indices[i], ESplineCoordinateSpace::World);
		const auto VecEndPos = Spline->GetLocationAtSplinePoint(Indices[i + 1], ESplineCoordinateSpace::World);
		const auto VecEndDirection = Spline->GetTangentAtSplinePoint(Indices[i + 1], ESplineCoordinateSpace::World);
		SplineMesh->SetStartAndEnd(VecStartPos, VecStartDirection, VecEndPos, VecEndDirection);
	}
}

/**
 * @brief update orbit maintaining the characteristics as best as possible
 * @param ALPHA gravitational parameter
 * @param RMAX maximum distance where stuff is simulated
 */
void AOrbit::UpdateOrbit(float ALPHA, float RMAX)
{
	UpdateOrbit(VecVelocity, ALPHA, RMAX);
}

void AOrbit::SetupActorInSpace(AActor* _ActorInSpace, FVector _VecF1, FVector VecV)
{
	ActorInSpace = _ActorInSpace;
	VecF1 = _VecF1;
	VecVelocity = VecV;
}

