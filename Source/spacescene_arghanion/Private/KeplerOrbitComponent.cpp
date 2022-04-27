// Fill out your copyright notice in the Description page of Project Settings.


#include "KeplerOrbitComponent.h"

#include "FunctionLib.h"
#include "MyGameInstance.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

void UKeplerOrbitComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	float DeltaR = 0;
	const auto VecRKepler = Body->GetComponentLocation() - VecF1;
	const auto R = VecRKepler.Length();
	const auto MU = GetOwner()->GetGameInstance<UMyGameInstance>()->MU;
	switch(Orbit)
	{
	case orbit::CIRCLE:
		DeltaR = VelocityScalar * DeltaTime;
		break;
	case orbit::ELLIPSE:
	 	VelocityScalar = UFunctionLib::VelocityEllipse(R, A, MU);
	 	DeltaR = VelocityScalar * DeltaTime;
		break;
	case orbit::LINEBOUND:
		VelocityScalar = UFunctionLib::VelocityEllipse(R, A, MU);
		DeltaR = VelocityScalar * DeltaTime;
		break;
	case orbit::LINEUNBOUND:
		VelocityScalar = VelocityScalar - copysign(MU / pow(R, 2) * DeltaTime, Velocity.Dot(VecRKepler));
		DeltaR = VelocityScalar * DeltaTime;
		break;
	case orbit::PARABOLA:
	 	VelocityScalar = UFunctionLib::VelocityParabola(R, MU);
		DeltaR = VelocityScalar * DeltaTime;
		break;
	case orbit::HYPERBOLA:
	 	VelocityScalar = UFunctionLib::VelocityEllipse(R, A, MU);
		DeltaR = VelocityScalar * DeltaTime;
		break;
	}
	VelocityNormalized = VelocityScalar / sqrt(MU / R);
	const auto NewSplineDistance = SplineDistance + DeltaR;
	if(IsClosedLoop())
	{
		SplineDistance = fmod(NewSplineDistance, GetSplineLength());
	}
	else if(NewSplineDistance > GetSplineLength())
	{
		SplineDistance = GetSplineLength();
	}
	else
	{
		SplineDistance = NewSplineDistance;
	}
	Velocity = GetTangentAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World).GetSafeNormal() * VelocityScalar;
	const auto NewLocation = GetLocationAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World);
	Body->SetWorldLocation(NewLocation);
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UKeplerOrbitComponent::UpdateOrbit(FVector VecR, FVector VecV, float MU, float RMAX)
{
	SetWorldLocation(VecF1);
	Velocity = VecV;
	VelocityScalar = VecV.Length();
	
	ClearSplinePoints(false);

	// transform location vector r to Kepler coordinates, where F1 is the origin
	const auto VecRKepler = VecR - VecF1;
	const auto R = VecRKepler.Length();
	VelocityNormalized = VelocityScalar / sqrt(MU / R);

	// the bigger this value, the earlier an eccentricity close to 1 will be interpreted as parabola orbit
	constexpr auto Tolerance = 1E-2;
	const auto VecH = VecRKepler.Cross(VecV);
	const float P = VecH.SquaredLength() / MU;
	const auto VecE = UFunctionLib::Eccentricity(VecRKepler, VecV, MU);
	Eccentricity = VecE.Length();
	const auto VecENorm = VecE.GetSafeNormal();
	const auto VecHNorm = VecH.GetSafeNormal();

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
			SplineDistance = GetDistanceAlongSplineAtSplineInputKey(FindInputKeyClosestToWorldLocation(VecR));
			SetClosedLoop(true, false);
			UpdateSpline();
			
			Orbit = orbit::LINEBOUND;
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
			UpdateSpline();
			SplineDistance = GetDistanceAlongSplineAtSplineInputKey(FindInputKeyClosestToWorldLocation(VecR));
			
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
		UpdateSpline();
		SplineDistance = GetDistanceAlongSplineAtSplineInputKey(FindInputKeyClosestToWorldLocation(VecR));
		Orbit = orbit::CIRCLE;
		A = R;
		Period = UFunctionLib::PeriodEllipse(R, MU);
	}

	// 0 < E < 1, Ellipse
	else if(Eccentricity <= 1. - Tolerance)
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
		UpdateSpline();
		SplineDistance = GetDistanceAlongSplineAtSplineInputKey(FindInputKeyClosestToWorldLocation(VecR));
		Orbit = orbit::ELLIPSE;
		Period = UFunctionLib::PeriodEllipse(A, MU);
	}
	
	// E = 1, Parabola
	else if(Eccentricity <= 1 + Tolerance)
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
		UpdateSpline();
		SplineDistance = GetDistanceAlongSplineAtSplineInputKey(FindInputKeyClosestToWorldLocation(VecR));
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
		UpdateSpline();
		SplineDistance = GetDistanceAlongSplineAtSplineInputKey(FindInputKeyClosestToWorldLocation(VecR));
		Orbit = orbit::HYPERBOLA;
		Period = 0;
	}

	VisualTrajectory->ClearInstances();
	constexpr auto Delta = 12.;
	for(int i = 0; i < GetSplineLength() / Delta; i++)
	{
		VisualTrajectory->AddInstance(GetTransformAtDistanceAlongSpline(i * Delta, ESplineCoordinateSpace::World), true);
	}
}

/**
 * @brief update orbit maintaining the characteristics as best as possible
 * @param MU gravitational parameter
 * @param RMAX maximum distance where stuff is simulated
 */
void UKeplerOrbitComponent::UpdateOrbit(const float MU, const float RMAX)
{
	const auto VecR = Body->GetComponentLocation();
	UpdateOrbit(VecR, Velocity, MU, RMAX);
}

void UKeplerOrbitComponent::Initialize(FVector _VecF1, TObjectPtr<USceneComponent> _Body, TObjectPtr<UHierarchicalInstancedStaticMeshComponent> _VisualTrajectory)
{
	VecF1 = _VecF1;
	Body = _Body;
	Velocity = FVector::Zero();
	VisualTrajectory = _VisualTrajectory;
}

void UKeplerOrbitComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedChainEvent)
{
	const auto Name = PropertyChangedChainEvent.PropertyChain.GetHead()->GetValue()->GetFName();
	static const FName FNameVelocityScalar = GET_MEMBER_NAME_CHECKED(UKeplerOrbitComponent, VelocityScalar);
	static const FName FNameVelocityNormalized = GET_MEMBER_NAME_CHECKED(UKeplerOrbitComponent, VelocityNormalized);
	static const FName FNameVelocity = GET_MEMBER_NAME_CHECKED(UKeplerOrbitComponent, Velocity);
	static const FName FNameSplineDistance = GET_MEMBER_NAME_CHECKED(UKeplerOrbitComponent, SplineDistance);
	static const FName FNameEccentricity = GET_MEMBER_NAME_CHECKED(UKeplerOrbitComponent, Eccentricity);
	
	const auto VecR = Body->GetComponentLocation();
	constexpr auto MU = DefaultMU;
	const auto RMAX = UMyGameInstance::DefaultGameAreaRadius;
	const auto VecRKepler = VecR - VecF1;
	FVector NewVelocity;
	
	const auto VelocityNormal = Velocity.GetUnsafeNormal();
	
	if(Name == FNameVelocityScalar)
	{
		NewVelocity = VelocityScalar * VelocityNormal;
		UpdateOrbit(VecR, NewVelocity, MU, RMAX);
	}
	else if(Name == FNameVelocityNormalized)
	{
		NewVelocity = sqrt(MU / VecRKepler.Length()) * VelocityNormalized * VelocityNormal;
		UpdateOrbit(VecR, NewVelocity, MU, RMAX);
	}
	else if(Name == FNameVelocity)
	{
		NewVelocity = Velocity;
		UpdateOrbit(VecR, NewVelocity, MU, RMAX);
	}
	else if(Name == FNameSplineDistance)
	{
		const auto R = VecRKepler.Length();
		SplineDistance = fmod(SplineDistance, GetSplineLength());
		if(SplineDistance < 0)
		{
			SplineDistance += GetSplineLength();
		}
		Velocity = GetTangentAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World).GetSafeNormal() * VelocityScalar;
		VelocityNormalized = VelocityScalar / sqrt(MU / R);
		VelocityScalar = Velocity.Length();
		const auto NewLocation = GetLocationAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World);
		Body->SetWorldLocation(NewLocation);
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
	Super::PostEditChangeChainProperty(PropertyChangedChainEvent);
}
