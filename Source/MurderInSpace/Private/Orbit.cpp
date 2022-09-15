#include "Orbit.h"

#include <numeric>

#include "FunctionLib.h"
#include "MyGameInstance.h"
#include "OrbitDataComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/SplineMeshComponent.h"

AOrbit::AOrbit()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	Spline->SetupAttachment(Root);
	Spline->ClearSplinePoints();

	HISMTrajectory = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("HISMTrajectory"));
	HISMTrajectory->SetupAttachment(Root);
}

void AOrbit::Update(float Alpha, float WorldRadius, FVector VecF1)
{
	if(!OrbitData)
	{
		// RequestEngineExit(TEXT("AOrbit::UpdateParameters: OrbitData null"));
		UE_LOG(LogActor, Error, TEXT("AOrbit::Update: OrbitData null. Not doing anything"))
		return;
	}
	
	Spline->ClearSplinePoints(false);

	// transform location vector r to Kepler coordinates, where F1 is the origin
	const auto VecRKepler = OrbitData->GetVecR() - VecF1;
	const auto RKepler = VecRKepler.Length();
	const auto VecVelocity = OrbitData->GetVecVelocity();
	//const auto VelocityVCircle = Velocity / sqrt(Alpha / R);

	// the bigger this value, the earlier an eccentricity close to 1 will be interpreted as parabola orbit
	constexpr auto Tolerance = 1E-2;
	const auto VecH = VecRKepler.Cross(VecVelocity);
	Params.P = VecH.SquaredLength() / Alpha;
	Params.Energy = pow(OrbitData->GetVelocity(), 2) / 2. - Alpha / RKepler;
	const auto VecE = UFunctionLib::Eccentricity(VecRKepler, VecVelocity, Alpha);
	Params.Eccentricity = VecE.Length();
	const auto VecENorm = VecE.GetSafeNormal();
	const auto VecHNorm = VecH.GetSafeNormal();

	// the energy of the weakest bound state: a circular orbit at R_MAX
	const auto E_BOUND_MIN = -Alpha / (2 * WorldRadius);

	// H = 0 (implies E = 1, too): falling in a straight line
	if(VecHNorm.IsZero())
	{
		const auto EMIN = -Alpha / WorldRadius;
		const auto E = VecVelocity.SquaredLength() / 2. - Alpha / RKepler;

		// bound
		if(E < EMIN)
		{
			const auto Apsis = -Alpha / E;
			const auto VecVNorm = VecVelocity.GetSafeNormal();
			if(VecVNorm.IsZero())
			{
				Spline->AddPoints(
					{ FSplinePoint(0, OrbitData->GetVecR(), ESplinePointType::Linear)
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
			DistanceZero = Spline->GetDistanceAlongSplineAtSplineInputKey(Spline->FindInputKeyClosestToWorldLocation(OrbitData->GetVecR()));
			SplineDistance = DistanceZero;
			Spline->SetClosedLoop(true, false);
			
			Params.OrbitType = EOrbitType::LINEBOUND;
			Params.A = UFunctionLib::SemiMajorAxis(VecRKepler, VecVelocity, Alpha);
			Params.Period = UFunctionLib::PeriodEllipse(Params.A, Alpha);
		}

		// unbound
		else
		{
			Spline->AddPoints(
				{ FSplinePoint(0, OrbitData->GetVecR(), ESplinePointType::Linear)
					, FSplinePoint(1, VecVelocity.GetUnsafeNormal() * (WorldRadius - OrbitData->GetVecR().Length()), ESplinePointType::Linear)
				});
			Spline->SetClosedLoop(false, false);
			Params.OrbitType = EOrbitType::LINEUNBOUND;
			Params.A = 0;
			Params.Period = 0;
		}
	}
	
	// E = 0, circle
	else if(VecENorm.IsZero())
	{
	    const auto VecP2 = VecHNorm.Cross(VecRKepler);
	    const auto VecT1 = VecHNorm.Cross(VecRKepler) * SplineToCircle;
	    const auto VecT4 = VecRKepler * SplineToCircle;
		Spline->AddPoints(
	      { FSplinePoint(0,  OrbitData->GetVecR(),  VecT1,  VecT1)
		    , FSplinePoint(1, VecP2 + VecF1, -VecT4, -VecT4)
		    , FSplinePoint(2, -VecRKepler + VecF1 , -VecT1, -VecT1)
		    , FSplinePoint(3, -VecP2 + VecF1,  VecT4,  VecT4)
		    });
    	Spline->SetClosedLoop(true, false);
		Params.OrbitType = EOrbitType::CIRCLE;
		Params.A = RKepler;
		Params.Period = UFunctionLib::PeriodEllipse(RKepler, Alpha);
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
	else if(Params.Eccentricity <= 1. - Tolerance || (Params.Eccentricity <= 1 && Params.Energy < E_BOUND_MIN))
	{
		Params.A = Params.P / (1 - VecE.SquaredLength());
	    const auto B = Params.A * sqrt(1 - VecE.SquaredLength());
	    const auto Vertex1 = Params.A * (1 - VecE.Length()) * VecENorm;
	    const auto Vertex2 = Params.A * (1 + VecE.Length()) * -VecENorm;
	    const auto Orthogonal = VecH.Cross(VecE).GetSafeNormal();
	    const auto Covertex1 =  B * Orthogonal - Params.A * VecE;
	    const auto Covertex2 = -B * Orthogonal - Params.A * VecE;
	    const auto T1 = Orthogonal * SplineToCircle * B;
	    const auto T4 = VecENorm * SplineToCircle * Params.A;
		Spline->AddPoints(
	      { FSplinePoint(0, Vertex1   + VecF1,  T1,  T1)
		    , FSplinePoint(1, Covertex1 + VecF1, -T4, -T4)
		    , FSplinePoint(2, Vertex2   + VecF1, -T1, -T1)
		    , FSplinePoint(3, Covertex2 + VecF1,  T4,  T4)
		    });
    	Spline->SetClosedLoop(true, false);
		Params.OrbitType = EOrbitType::ELLIPSE;
		Params.Period = UFunctionLib::PeriodEllipse(Params.A, Alpha);
	}
	
	// E = 1, Parabola
	else if(Params.Eccentricity <= 1. + Tolerance)
	{
		std::list<FVector> Points;
	    const auto VecHorizontal = VecHNorm.Cross(VecENorm);
		constexpr auto MAX_POINTS = 20;
		const auto MAX_N = sqrt(2 * (WorldRadius + VecF1.Length()) / Params.P);
		const auto Delta = 2 * MAX_N / MAX_POINTS;
		
		Points.emplace_front(VecENorm * Params.P / (1. + Params.Eccentricity) + VecF1);
		for(int i = 1; i < MAX_POINTS / 2; i++)
		{
			const auto VecX = i * Delta * VecHorizontal * Params.P;
			const auto VecY = VecENorm / 2. * (1 - pow(i * Delta, 2)) * Params.P;
			Points.emplace_back(VecY + VecX + VecF1);
			Points.emplace_front(VecY - VecX + VecF1);
		}
		
		for(auto Point : Points)
		{
			Spline->AddSplineWorldPoint(Point);
		}
    	Spline->SetClosedLoop(false, false);
		Params.OrbitType = EOrbitType::PARABOLA;
		Params.Period = 0;
		Params.A = 0;
	}

	// E > 1, Hyperbola
	else
	{
		std::list<FVector> Points;
		Params.A = Params.P / (1 - VecE.SquaredLength()); // A < 0
		const auto C = Params.P * Params.Eccentricity / (VecE.SquaredLength() - 1);
	    const auto VecHorizontal = VecHNorm.Cross(VecENorm);
		constexpr auto MAX_POINTS = 20;
		const auto MAX = sqrt((pow(WorldRadius, 2) + (VecE.SquaredLength() - 1.) * pow(Params.A, 2)) / pow(Params.Eccentricity, 2));
		const auto Delta = 2. * MAX / (pow(MAX_POINTS / 2 - 1, 3) / 3.);

		Points.emplace_front(VecENorm * Params.P / (1. + Params.Eccentricity) + VecF1);
		//Points.emplace_front(VecE * A + VecF1);
		for(int i = 1; i < MAX_POINTS / 2; i++)
		{
			const auto X = pow(i, 2) * Delta - Params.A;
			const auto VecX = (C - X) * VecENorm;
			//(P - sqrt(VecE.SquaredLength() - 1) * RMAX) * VecENorm + VecF1;	
			//const auto VecY = VecHorizontal * sqrt((VecE.SquaredLength() - 1.) * pow(X, 2) - 1.);
			const auto VecY = VecHorizontal * sqrt((VecE.SquaredLength() - 1.) * (pow(X, 2) - pow(Params.A, 2)));
			Points.emplace_back(VecX + VecY + VecF1);
			Points.emplace_front(VecX - VecY + VecF1);
		}
			
		for(auto Point : Points)
		{
			Spline->AddSplineWorldPoint(Point);
		}
    	Spline->SetClosedLoop(false, false);
		Params.OrbitType = EOrbitType::HYPERBOLA;
		Params.Period = 0;
	}
	Spline->UpdateSpline();

	if(Params.OrbitType != EOrbitType::LINEBOUND)
	{
		SplineKey = Spline->FindInputKeyClosestToWorldLocation(OrbitData->GetVecR());
		DistanceZero = Spline->GetDistanceAlongSplineAtSplineInputKey(SplineKey);
		SplineDistance = DistanceZero;
	}
	// else
	// `SplineKey` is not needed,
	// `SplineDistance` and `DistanceZero` are set already

	if(bTrajectoryShowSpheres)
	{
		const auto HISMLength = std::min<float>(HISMMaxLength, Spline->GetSplineLength());
		HISMNumberOfMarkers = std::max<int>(1, HISMLength / HISMDistance);

		HISMTrajectory->ClearInstances();
		for(int i = 1; i < HISMNumberOfMarkers; i++)
		{
			const auto Transform = Spline->GetTransformAtDistanceAlongSpline(DistanceZero + i * HISMDistance, ESplineCoordinateSpace::World);
			HISMTrajectory->AddInstance(Transform);
		}
		HISMCurrentIndex = 0;
	}

	if(bTrajectoryShowSpline)
	{
		TArray<USplineMeshComponent*> OldSplinesMeshes;
		GetComponents(OldSplinesMeshes);
		for(auto Old : OldSplinesMeshes)
		{
			Old->DestroyComponent();
		}

		const auto nIndices = static_cast<int>(round(Spline->GetSplineLength() / splineMeshLength));
		if(nIndices >= 2)
		{
			std::vector<int> Indices(nIndices);
			std::iota(Indices.begin(), Indices.end(), 0);

			if(Spline->IsClosedLoop() && Params.OrbitType != EOrbitType::LINEBOUND)
			{
				Indices.push_back(0);
			}
			for(int i = 0; i < Indices.size() - 1; i++)
			{
				const auto SplineMesh = NewObject<USplineMeshComponent>(this, *FString(TEXT("SplineMesh")).Append(FString::FromInt(i)));
				// if I don't register here, the spline mesh doesn't render
				SplineMesh->RegisterComponent();
				SplineMesh->AttachToComponent(Root, FAttachmentTransformRules::KeepWorldTransform);
				// if I don't add instance here, the spline meshes don't show in the component list in the editor
				AddInstanceComponent(SplineMesh);
				
				SplineMesh->SetMobility(EComponentMobility::Stationary);
				SplineMesh->CastShadow = false;
				SplineMesh->SetStaticMesh(SM_Trajectory);
				SplineMesh->SetMaterial(0, OrbitData->SplineMeshMaterial);
				const auto VecStartPos = Spline->GetLocationAtDistanceAlongSpline(Indices[i] * splineMeshLength, ESplineCoordinateSpace::World);
				const auto VecStartDirection = Spline->GetTangentAtDistanceAlongSpline(Indices[i] * splineMeshLength, ESplineCoordinateSpace::World).GetUnsafeNormal() * splineMeshLength;
				const auto VecEndPos = Spline->GetLocationAtDistanceAlongSpline(Indices[i + 1] * splineMeshLength, ESplineCoordinateSpace::World);
				const auto VecEndDirection = Spline->GetTangentAtDistanceAlongSpline(Indices[i + 1] * splineMeshLength, ESplineCoordinateSpace::World).GetUnsafeNormal() * splineMeshLength;
				SplineMesh->SetStartAndEnd(VecStartPos, VecStartDirection, VecEndPos, VecEndDirection);
			}
		}
	}
	bInitialized = true;
}

float AOrbit::VelocityEllipse(float R, float Alpha)
{
    return std::max(sqrt(Alpha * (2.0 / R - 1.0 / Params.A)), 1.);
}

float AOrbit::VelocityParabola(float R, float Alpha)
{
    return sqrt(Alpha * 2.0 / R);
}

float AOrbit::NextVelocity(float R, float Alpha, float OldVelocity, float DeltaTime, float Sign)
{
	if(!bInitialized)
	{
		RequestEngineExit(TEXT("AOrbit::NextVelocity: not initialized"));
	}
	
	switch(Params.OrbitType)
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

FNewVelocityAndLocation AOrbit::AdvanceOnSpline(float DeltaR, float Velocity, FVector VecR, float DeltaTime)
{
	SplineDistance = fmod(SplineDistance + DeltaR, Spline->GetSplineLength());
	if(Params.OrbitType == EOrbitType::LINEBOUND)
	{
		const auto NewLocation = Spline->GetLocationAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World);
		const auto NewVelocity = Spline->GetTangentAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World).GetSafeNormal() * Velocity;
		return { NewVelocity, NewLocation};
	}
	else
	{
		// direction at current position, i.e. at current spline key
		const auto NewVelocity = Spline->GetTangentAtSplineInputKey(SplineKey, ESplineCoordinateSpace::World).GetSafeNormal() * Velocity;
		const auto NewLocationAtTangent = VecR + NewVelocity * DeltaTime;
		
		// new spline key
		SplineKey = Spline->FindInputKeyClosestToWorldLocation(NewLocationAtTangent);
		const auto NewLocation = Spline->GetLocationAtSplineInputKey(SplineKey, ESplineCoordinateSpace::World);

		// update trajectory HISM markers
		if(bTrajectoryShowSpheres && (SplineDistance - DistanceZero) / HISMDistance > HISMCurrentIndex)
		{
			const auto SplineLength = std::max<float>(HISMDistance, Spline->GetSplineLength());
			const auto Distance = fmod(DistanceZero + (HISMNumberOfMarkers + HISMCurrentIndex) * HISMDistance, SplineLength);
			const auto Transform = Spline->GetTransformAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
			HISMTrajectory->UpdateInstanceTransform(HISMCurrentIndex % HISMNumberOfMarkers, Transform, true);
			HISMCurrentIndex = (HISMCurrentIndex + 1) % static_cast<int>(std::round(SplineLength / HISMDistance));
		}
		return { NewVelocity, NewLocation };
	}
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
	return StrOrbitType + FString::Printf(TEXT(", E = %.2f, P = %.1f, Energy = %.1f, Period = %.1f, A = %.1f"), Params.Eccentricity, Params.P, Params.Energy, Params.Period, Params.A);
}

#if WITH_EDITOR
void AOrbit::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
	
	const auto WorldRadius = UMyGameInstance::EditorDefaultWorldRadiusUU;
	const auto Alpha = UMyGameInstance::EditorDefaultAlpha;
	const auto VecF1 = UMyGameInstance::EditorDefaultVecF1;
	
	const auto Name = PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName();

	static const FName FNameBTrajectoryShowSpline = GET_MEMBER_NAME_CHECKED(AOrbit, bTrajectoryShowSpline);
	static const FName FNameSplineMeshLength = GET_MEMBER_NAME_CHECKED(AOrbit, splineMeshLength);
	static const FName FNameBTrajectoryShowSpheres = GET_MEMBER_NAME_CHECKED(AOrbit, bTrajectoryShowSpheres);

	if(Name == FNameSplineMeshLength || Name == FNameBTrajectoryShowSpline || Name == FNameBTrajectoryShowSpheres)
	{
		Update(Alpha, WorldRadius, VecF1);
	}
}
#endif
