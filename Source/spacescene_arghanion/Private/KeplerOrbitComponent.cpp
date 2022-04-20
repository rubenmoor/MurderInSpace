// Fill out your copyright notice in the Description page of Project Settings.


#include "KeplerOrbitComponent.h"

#include "FunctionLib.h"
#include "MyGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UKeplerOrbitComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	float DeltaR = 0;
	FVector VecRKepler;
	float R;
	const auto MU = GetOwner()->GetGameInstance<UMyGameInstance>()->MU;
	switch(Orbit)
	{
	case orbit::CIRCLE:
		DeltaR = VelocityScalar * DeltaTime;
		break;
	case orbit::ELLIPSE:
		VecRKepler = Body->GetComponentLocation() - VecF1;
	 	R = VecRKepler.Length();
	 	VelocityScalar = UFunctionLib::Velocity(R, A, MU);
		VelocityNormalized = VelocityScalar / sqrt(MU / R);
	 	DeltaR = VelocityScalar * DeltaTime;
		break;
	case orbit::PARABOLA:
		DeltaR = VelocityScalar * DeltaTime;
		break;
	case orbit::HYPERBOLA:
		DeltaR = VelocityScalar * DeltaTime;
		break;
	}
	SplineDistance = fmod(SplineDistance + DeltaR, GetSplineLength());
	Velocity = GetTangentAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World).GetSafeNormal() * VelocityScalar;
	const auto NewLocation = GetLocationAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World);
	Body->SetWorldLocation(NewLocation);
	if(isnan(SplineDistance))
	{
		UE_LOG(LogTemp, Error, TEXT("Tick: SplineDistance: nan, DeltaR: %f, Spline length: %f"), DeltaR, GetSplineLength());
	}
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UKeplerOrbitComponent::BeginPlay()
{
	const auto GameInstance = GetOwner()->GetGameInstance<UMyGameInstance>();
	if(!GameInstance) RequestEngineExit(TEXT("GameInstance cast to UMyGameInstance: failed"));
	Super::BeginPlay();
}

void UKeplerOrbitComponent::UpdateOrbit(FVector VecR, FVector VecV, float MU)
{
	SetWorldLocation(VecF1);
	Velocity = VecV;
	VelocityScalar = VecV.Length();
	
	ClearSplinePoints(false);

	// transform location vector r to Kepler coordinates, where F1 is the origin
	const auto VecRKepler = VecR - VecF1;
	const auto R = VecRKepler.Length();
	VelocityNormalized = VelocityScalar / sqrt(MU / R);
	
	constexpr auto Tolerance = 1E-8;
	const auto VecH = VecRKepler.Cross(VecV);
	const float P = VecH.SquaredLength() / MU;
	const auto VecE = UFunctionLib::Eccentricity(VecRKepler, VecV, MU);
	const auto E = VecE.Length();
	const auto VecENorm = VecE.GetSafeNormal(Tolerance);
	const auto VecHNorm = VecH.GetSafeNormal();

	// E = 0, circle
	if(VecENorm.IsZero())
	{
	    const auto VecP2 = VecHNorm.Cross(VecRKepler);
	    const auto VecT1 = VecHNorm.Cross(VecRKepler) * SplineToCircle;
	    const auto VecT4 = VecRKepler * SplineToCircle;
		AddPoints(
	      { FSplinePoint(0,  VecR ,  VecT1,  VecT1)
		    , FSplinePoint(1,  VecP2 + VecF1, -VecT4, -VecT4)
		    , FSplinePoint(2, -VecRKepler + VecF1 , -VecT1, -VecT1)
		    , FSplinePoint(3, -VecP2 + VecF1,  VecT4,  VecT4)
		    });
		Orbit = orbit::CIRCLE;
		A = R;
		Period = UFunctionLib::PeriodEllipse(R, MU);
	}

	// E = 1, Parabola
	else if(abs(E - 1) < Tolerance)
	{
	    const auto VecHorizontal = VecHNorm.Cross(VecENorm);
	    // TODO: check if tangents need setup
	    AddPoint(FSplinePoint(0, VecENorm * P / 2 + VecF1));
	    auto parabola = [VecHorizontal, VecENorm, P, this](int n, int sign)
	    {
		    return (sign * n * VecHorizontal + VecENorm / 2.0 * ( 1 - pow(n, 2))) * P + VecF1;
	    };
	    for(int i = 1; i < 10; i++)
	    {
		    AddPoint(FSplinePoint(2.0 * i - 1, parabola(i, 1)));
		    AddPoint(FSplinePoint(2.0 * i    , parabola(i, -1)));
	    }
		Orbit = orbit::PARABOLA;
		Period = 0;
	}

	// E > 1, Hyperbola
	else if(E > 1)
	{
		//const float A = P / (VecE.SquaredLength() - 1);
		//const float B = P / sqrt(VecE.SquaredLength() - 1);
		Orbit = orbit::HYPERBOLA;
		// TODO
	}
	
	// 0 < E < 1, Ellipse
	else
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
	      { FSplinePoint(0, Vertex1  ,  T1,  T1)
		    , FSplinePoint(1, Covertex1, -T4, -T4)
		    , FSplinePoint(2, Vertex2  , -T1, -T1)
		    , FSplinePoint(3, Covertex2,  T4,  T4)
		    });
		Orbit = orbit::ELLIPSE;
		Period = UFunctionLib::PeriodEllipse(A, MU);
	}
	
	UpdateSpline();
	SplineDistance = GetDistanceAlongSplineAtSplineInputKey(FindInputKeyClosestToWorldLocation(VecR));
}

/**
 * @brief update orbit maintaining the characteristics as best as possible
 * @param MU graviational parameter
 */
void UKeplerOrbitComponent::UpdateOrbit(float MU)
{
	const auto VecR = Body->GetComponentLocation();
	FVector VecV;
	if(Velocity.IsZero())
	{
		const auto VecRKepler = VecR - VecF1;
		const auto VelocityNormal = FVector(0,0,1).Cross(VecRKepler).GetUnsafeNormal();
		VecV = sqrt(MU / VecRKepler.Length()) * VelocityNormal;
	}
	else
	{
		VecV = Velocity;
	}
	this->UpdateOrbit(VecR, VecV, MU);
}

void UKeplerOrbitComponent::Initialize(FVector _VecF1, USceneComponent* _Body)
{
	VecF1 = _VecF1;
	Body = _Body;
	Velocity = FVector::Zero();
}

void UKeplerOrbitComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedChainEvent)
{
	const auto Name = PropertyChangedChainEvent.PropertyChain.GetHead()->GetValue()->GetFName();
	static const FName FNameVelocityScalar = GET_MEMBER_NAME_CHECKED(UKeplerOrbitComponent, VelocityScalar);
	static const FName FNameVelocityNormalized = GET_MEMBER_NAME_CHECKED(UKeplerOrbitComponent, VelocityNormalized);
	static const FName FNameVelocity = GET_MEMBER_NAME_CHECKED(UKeplerOrbitComponent, Velocity);
	static const FName FNameSplineDistance = GET_MEMBER_NAME_CHECKED(UKeplerOrbitComponent, SplineDistance);
	
	const auto VecR = Body->GetComponentLocation();
	const auto MU = DefaultMU;
	const auto VecRKepler = VecR - VecF1;
	FVector VelocityNormal;
	FVector NewVelocity;
	
	if(Velocity.Length() < 1e-8)
	{
		if(VecRKepler.IsZero())
		{
			UE_LOG(LogTemp, Error, TEXT("VecRKepler zero, unexpected"));
		}
		VelocityNormal = FVector(0,0,1).Cross(VecRKepler.GetUnsafeNormal());
		//NewVelocity = sqrt(MU / VecRKepler.Length()) * VelocityNormal;
	}
	else
	{
		VelocityNormal = Velocity.GetUnsafeNormal();
	}
	
	if(Name == FNameVelocityScalar)
	{
		NewVelocity = VelocityScalar * VelocityNormal;
		UpdateOrbit(VecR, NewVelocity, MU);
	}
	else if(Name == FNameVelocityNormalized)
	{
		NewVelocity = sqrt(MU / VecRKepler.Length()) * VelocityNormalized * VelocityNormal;
		UpdateOrbit(VecR, NewVelocity, MU);
	}
	else if(Name == FNameVelocity)
	{
		NewVelocity = Velocity;
		UpdateOrbit(VecR, NewVelocity, MU);
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
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("In UKeplerOrbitComponent::PostEditChangeProperty: %s, not doing anything"), *Name.ToString());
	}
	Super::PostEditChangeChainProperty(PropertyChangedChainEvent);
}
