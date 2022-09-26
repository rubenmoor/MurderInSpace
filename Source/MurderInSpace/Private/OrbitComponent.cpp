// Fill out your copyright notice in the Description page of Project Settings.


#include "OrbitComponent.h"

#include <algorithm>
#include <numeric>

#include "FunctionLib.h"
#include "MyGameInstance.h"
#include "Components/SplineMeshComponent.h"

UOrbitComponent::UOrbitComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	USceneComponent::SetMobility(EComponentMobility::Stationary);
	
	// debugging: trying to find source of error:
	// Ensure condition failed: !Primitive->Bounds.ContainsNaN() [File:D:\build\++UE5\Sync\Engine\Source\Runtime\Renderer\Private\RendererScene.cpp] [Line: 1365]
	if(Bounds.ContainsNaN())
	{
		UE_LOG(LogActorComponent, Error, TEXT("%s: Constructor: contains NaN"), *GetFullName())
	}
}

void UOrbitComponent::UpdateWithParams(float Alpha, float WorldRadius, FVector VecF1)
{
	if(!MovableRoot)
	{
		UE_LOG(LogActorComponent, Error, TEXT("%s: Update: MovableRoot null; not doing anything"), *GetFullName())
		return;
	}
	ClearSplinePoints(false);

	// transform location vector r to Kepler coordinates, where F1 is the origin
	const FVector VecRKepler = VecR - VecF1;
	const float RKepler = VecRKepler.Length();
	//const auto VelocityVCircle = Velocity / sqrt(Alpha / R);

	// the bigger this value, the earlier an eccentricity close to 1 will be interpreted as parabola orbit
	constexpr float Tolerance = 1E-2;
	const FVector VecH = VecRKepler.Cross(VecVelocity);
	Params.P = VecH.SquaredLength() / Alpha;
	Params.Energy = pow(Velocity, 2) / 2. - Alpha / RKepler;
	const FVector VecE = UFunctionLib::Eccentricity(VecRKepler, VecVelocity, Alpha);
	Params.Eccentricity = VecE.Length();
	const FVector VecENorm = VecE.GetSafeNormal();
	const FVector VecHNorm = VecH.GetSafeNormal();

	// the energy of the weakest bound state: a circular orbit at R_MAX
	const float E_BOUND_MIN = -Alpha / (2 * WorldRadius);

	// H = 0 (implies E = 1, too): falling in a straight line
	if(VecHNorm.IsZero())
	{
		const float EMIN = -Alpha / WorldRadius;
		const float E = VecVelocity.SquaredLength() / 2. - Alpha / RKepler;

		// bound
		if(E < EMIN)
		{
			const float Apsis = -Alpha / E;
			const FVector VecVNorm = VecVelocity.GetSafeNormal();
			if(VecVNorm.IsZero())
			{
				MyAddPoints(
					{ FSplinePoint(0, VecR, ESplinePointType::Linear)
					, FSplinePoint(1, -VecRKepler + VecF1, ESplinePointType::Linear)
					}, false);
			}
			else
			{
				MyAddPoints(
					{ FSplinePoint(0, -VecVNorm * Apsis + VecF1, ESplinePointType::Linear)
						, FSplinePoint(1, VecVNorm * Apsis + VecF1, ESplinePointType::Linear)
					}, false);
				SetClosedLoop(false, false);
				UpdateSpline();
			}
			DistanceZero = GetDistanceAlongSplineAtSplineInputKey(FindInputKeyClosestToWorldLocation(VecR));
			SplineDistance = DistanceZero;
			SetClosedLoop(true, false);
			
			Params.OrbitType = EOrbitType::LINEBOUND;
			Params.A = UFunctionLib::SemiMajorAxis(VecRKepler, VecVelocity, Alpha);
			Params.Period = UFunctionLib::PeriodEllipse(Params.A, Alpha);
		}

		// unbound
		else
		{
			MyAddPoints(
				{ FSplinePoint(0, VecR, ESplinePointType::Linear)
					, FSplinePoint(1, VecVelocity.GetUnsafeNormal() * (WorldRadius - VecR.Length()), ESplinePointType::Linear)
				}, false);
			SetClosedLoop(false, false);
			Params.OrbitType = EOrbitType::LINEUNBOUND;
			Params.A = 0;
			Params.Period = 0;
		}
	}
	
	// E = 0, circle
	else if(VecENorm.IsZero())
	{
	    const FVector VecP2 = VecHNorm.Cross(VecRKepler);
	    const FVector VecT1 = VecHNorm.Cross(VecRKepler) * SplineToCircle;
	    const FVector VecT4 = VecRKepler * SplineToCircle;
		MyAddPoints(
	      { FSplinePoint(0,  VecR,  VecT1,  VecT1)
		    , FSplinePoint(1, VecP2 + VecF1, -VecT4, -VecT4)
		    , FSplinePoint(2, -VecRKepler + VecF1 , -VecT1, -VecT1)
		    , FSplinePoint(3, -VecP2 + VecF1,  VecT4,  VecT4)
		    }, false);
    	SetClosedLoop(true, false);
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
	// For a black hole, a P cutoff isn't ideal (small periapses are allowed), thus instead I define a cutoff energy:
	// The total energy at R_MAX when orbiting in a circle is the bound state with energy closest to zero.
	// As long the energy is smaller than that, we can safely assume a bound state and thus an ellipse instead of
	// the parabola
	else if(Params.Eccentricity <= 1. - Tolerance || (Params.Eccentricity <= 1 && Params.Energy < E_BOUND_MIN))
	{
		Params.A = Params.P / (1 - VecE.SquaredLength());
	    const float B = Params.A * sqrt(1 - VecE.SquaredLength());
	    const FVector Vertex1 = Params.A * (1 - VecE.Length()) * VecENorm;
	    const FVector Vertex2 = Params.A * (1 + VecE.Length()) * -VecENorm;
	    const FVector Orthogonal = VecH.Cross(VecE).GetSafeNormal();
	    const FVector Covertex1 =  B * Orthogonal - Params.A * VecE;
	    const FVector Covertex2 = -B * Orthogonal - Params.A * VecE;
	    const FVector T1 = Orthogonal * SplineToCircle * B;
	    const FVector T4 = VecENorm * SplineToCircle * Params.A;
		MyAddPoints(
	      { FSplinePoint(0, Vertex1   + VecF1,  T1,  T1)
		    , FSplinePoint(1, Covertex1 + VecF1, -T4, -T4)
		    , FSplinePoint(2, Vertex2   + VecF1, -T1, -T1)
		    , FSplinePoint(3, Covertex2 + VecF1,  T4,  T4)
		    }, false);
    	SetClosedLoop(true, false);
		Params.OrbitType = EOrbitType::ELLIPSE;
		Params.Period = UFunctionLib::PeriodEllipse(Params.A, Alpha);
	}
	
	// E = 1, Parabola
	else if(Params.Eccentricity <= 1. + Tolerance)
	{
		std::list<FVector> Points;
	    const FVector VecHorizontal = VecHNorm.Cross(VecENorm);
		constexpr int MAX_POINTS = 20;
		const float MAX_N = sqrt(2 * (WorldRadius + VecF1.Length()) / Params.P);
		const float Delta = 2 * MAX_N / MAX_POINTS;
		
		Points.emplace_front(VecENorm * Params.P / (1. + Params.Eccentricity) + VecF1);
		for(int i = 1; i < MAX_POINTS / 2; i++)
		{
			const FVector VecX = i * Delta * VecHorizontal * Params.P;
			const FVector VecY = VecENorm / 2. * (1 - pow(i * Delta, 2)) * Params.P;
			Points.emplace_back(VecY + VecX + VecF1);
			Points.emplace_front(VecY - VecX + VecF1);
		}
		
		for(const FVector Point : Points)
		{
			AddSplineWorldPoint(Point);
		}
    	SetClosedLoop(false, false);
		Params.OrbitType = EOrbitType::PARABOLA;
		Params.Period = 0;
		Params.A = 0;
	}

	// E > 1, Hyperbola
	else
	{
		std::list<FVector> Points;
		Params.A = Params.P / (1 - VecE.SquaredLength()); // A < 0
		const float C = Params.P * Params.Eccentricity / (VecE.SquaredLength() - 1);
	    const FVector VecHorizontal = VecHNorm.Cross(VecENorm);
		constexpr int MAX_POINTS = 20;
		const float MAX = sqrt((pow(WorldRadius, 2) + (VecE.SquaredLength() - 1.) * pow(Params.A, 2)) / pow(Params.Eccentricity, 2));
		const float Delta = 2. * MAX / (pow(MAX_POINTS / 2 - 1, 3) / 3.);

		Points.emplace_front(VecENorm * Params.P / (1. + Params.Eccentricity) + VecF1);
		//Points.emplace_front(VecE * A + VecF1);
		for(int i = 1; i < MAX_POINTS / 2; i++)
		{
			const float X = pow(i, 2) * Delta - Params.A;
			const FVector VecX = (C - X) * VecENorm;
			//(P - sqrt(VecE.SquaredLength() - 1) * RMAX) * VecENorm + VecF1;	
			//const auto VecY = VecHorizontal * sqrt((VecE.SquaredLength() - 1.) * pow(X, 2) - 1.);
			const FVector VecY = VecHorizontal * sqrt((VecE.SquaredLength() - 1.) * (pow(X, 2) - pow(Params.A, 2)));
			Points.emplace_back(VecX + VecY + VecF1);
			Points.emplace_front(VecX - VecY + VecF1);
		}
			
		for(const FVector Point : Points)
		{
			AddSplineWorldPoint(Point);
		}
    	SetClosedLoop(false, false);
		Params.OrbitType = EOrbitType::HYPERBOLA;
		Params.Period = 0;
	}
	UpdateSpline();

	if(Params.OrbitType != EOrbitType::LINEBOUND)
	{
		SplineKey = FindInputKeyClosestToWorldLocation(VecR);
		DistanceZero = GetDistanceAlongSplineAtSplineInputKey(SplineKey);
		SplineDistance = DistanceZero;
	}
	// else
	// `SplineKey` is not needed,
	// `SplineDistance` and `DistanceZero` are set already
	
	TArray<USceneComponent*> OldSplinesMeshes;
	GetChildrenComponents(false, OldSplinesMeshes);
	for(USceneComponent* const Old : OldSplinesMeshes)
	{
		Old->DestroyComponent();
	}

	if(bTrajectoryShowSpline)
	{
		const int nIndices = static_cast<int>(round(GetSplineLength() / splineMeshLength));
		if(nIndices >= 2)
		{
			std::vector<int> Indices(nIndices);
			std::iota(Indices.begin(), Indices.end(), 0);

			if(IsClosedLoop() && Params.OrbitType != EOrbitType::LINEBOUND)
			{
				Indices.push_back(0);
			}
			for(int i = 0; i < Indices.size() - 1; i++)
			{
				const TObjectPtr<USplineMeshComponent> SplineMesh =
					NewObject<USplineMeshComponent>(this, *FString(TEXT("SplineMesh")).Append(FString::FromInt(i)));
				
				SplineMesh->SetMobility(EComponentMobility::Stationary);
				SplineMesh->SetVisibility(bIsVisible);
				
				// if I don't register here, the spline mesh doesn't render
				SplineMesh->RegisterComponent();
				SplineMesh->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
				// if I don't add instance here, the spline meshes don't show in the component list in the editor
				GetOwner()->AddInstanceComponent(SplineMesh);
				
				SplineMesh->CastShadow = false;
				SplineMesh->SetStaticMesh(SM_Trajectory);
				SplineMesh->SetMaterial(0, SplineMeshMaterial);
				const FVector VecStartPos = GetLocationAtDistanceAlongSpline(Indices[i] * splineMeshLength, ESplineCoordinateSpace::World);
				const FVector VecStartDirection = GetTangentAtDistanceAlongSpline(Indices[i] * splineMeshLength, ESplineCoordinateSpace::World).GetUnsafeNormal() * splineMeshLength;
				const FVector VecEndPos = GetLocationAtDistanceAlongSpline(Indices[i + 1] * splineMeshLength, ESplineCoordinateSpace::World);
				const FVector VecEndDirection = GetTangentAtDistanceAlongSpline(Indices[i + 1] * splineMeshLength, ESplineCoordinateSpace::World).GetUnsafeNormal() * splineMeshLength;
				SplineMesh->SetStartAndEnd(VecStartPos, VecStartDirection, VecEndPos, VecEndDirection);
			}
		}
	}
	bInitialized = true;
}

void UOrbitComponent::Update(UMyGameInstance* GI)
{
	UpdateWithParams(GI->Alpha, GI->WorldRadius, GI->VecF1);
}

float UOrbitComponent::VelocityEllipse(float R, float Alpha)
{
    return std::max(sqrt(Alpha * (2.0 / R - 1.0 / Params.A)), 1.);
}

float UOrbitComponent::VelocityParabola(float R, float Alpha)
{
    return sqrt(Alpha * 2.0 / R);
}

float UOrbitComponent::NextVelocity(float R, float Alpha, float OldVelocity, float DeltaTime, float Sign)
{
	if(!bInitialized)
	{
		UE_LOG(LogActorComponent, Error, TEXT("%s: NextVelocity: not initialized"), *GetFullName());
		return 0.;
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
		UE_LOG(LogTemp, Error, TEXT("UOrbitComponent::NextVelocity: Impossible"))
		return 0;
	}
}

// add spline points, add in world coordinates; however: this one only corrects
// for the translation, not for rotation and scale
void UOrbitComponent::MyAddPoints(TArray<FSplinePoint> InSplinePoints, bool bUpdateSpline)
{
	const FVector Loc = GetComponentLocation();
	for(FSplinePoint& p : InSplinePoints)
	{
		p.Position -= Loc;
	}
	AddPoints(InSplinePoints, bUpdateSpline);
}

// void UOrbitComponent::SetVisibility(bool InBVisible)
// {
// 	TArray<USceneComponent*> OldSplinesMeshes;
// 	GetChildrenComponents(false, OldSplinesMeshes);
// 	for(USceneComponent* const Old : OldSplinesMeshes)
// 	{
// 		Old->SetVisibility(InBVisible);
// 	}
// }

FString UOrbitComponent::GetParamsString()
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

float UOrbitComponent::GetCircleVelocity(float Alpha, FVector VecF1) const
{
	return sqrt(Alpha / (VecR - VecF1).Length());
}

void UOrbitComponent::SetVelocity(FVector _VecVelocity, float Alpha, FVector VecF1)
{
	VecVelocity = _VecVelocity;
	Velocity = VecVelocity.Length();
	VelocityVCircle = Velocity / GetCircleVelocity(Alpha, VecF1);
}

void UOrbitComponent::AddVelocity(FVector _VecVelocity, UMyGameInstance* GI)
{
	SetVelocity(VecVelocity + _VecVelocity, GI->Alpha, GI->VecF1);
	Update(GI);
}

void UOrbitComponent::InitializeCircle(float Alpha, float WorldRadius, FVector VecF1, FVector NewVecR)
{
	VecR = NewVecR;
	const FVector VecRKepler = VecR - VecF1;
	const FVector VelocityNormal = FVector(0., 0., 1.).Cross(VecR).GetSafeNormal(1e-8, FVector(0., 1., 0.));
	VelocityVCircle = 1.;
	Velocity = sqrt(Alpha / VecRKepler.Length()) * VelocityVCircle;
	VecVelocity = Velocity * VelocityNormal;
	UpdateWithParams(Alpha, WorldRadius, VecF1);
}

void UOrbitComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(!bInitialized)
	{
		RequestEngineExit(TEXT("UOrbitComponent::TickComponent: not initialized"));
		return;
	}
	
	const TObjectPtr<UMyGameInstance> GI = GetWorld()->GetGameInstance<UMyGameInstance>();
	const FVector VecF1 = GI->VecF1;
	const float Alpha = GI->Alpha;
	
	const FVector VecRKepler = VecR - VecF1;

	Velocity = NextVelocity(VecRKepler.Length(), Alpha, Velocity, DeltaTime, VecVelocity.Dot(VecRKepler));
	VelocityVCircle = Velocity / GetCircleVelocity(Alpha, VecF1);
	const float DeltaR = Velocity * DeltaTime;

	// advance on spline
	SplineDistance = fmod(SplineDistance + DeltaR, GetSplineLength());
	if(Params.OrbitType == EOrbitType::LINEBOUND)
	{
		VecR = GetLocationAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World);
		VecVelocity = GetTangentAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World).GetSafeNormal() * Velocity;
	}
	else
	{
		// direction at current position, i.e. at current spline key
		VecVelocity = GetTangentAtSplineInputKey(SplineKey, ESplineCoordinateSpace::World).GetSafeNormal() * Velocity;
		const FVector NewLocationAtTangent = VecR + VecVelocity * DeltaTime;
		
		// new spline key
		SplineKey = FindInputKeyClosestToWorldLocation(NewLocationAtTangent);
		VecR = GetLocationAtSplineInputKey(SplineKey, ESplineCoordinateSpace::World);
	}
	MovableRoot->SetWorldLocation(VecR, true, nullptr, ETeleportType::TeleportPhysics);
	
	// TODO: account for acceleration
	// const auto RealDeltaR = (GetVecR() - VecR).Length();
	// const auto RelativeError = DeltaR / RealDeltaR - 1.;
	// if(abs(RelativeError) > 0.02)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("%s: Expected: %f; really: %f; relative error: %.1f%"), *GetFName().ToString(), DeltaR, RealDeltaR, RelativeError * 100.);
	// }
}

void UOrbitComponent::BeginPlay()
{
	Super::BeginPlay();

	SetVisibility(false);
	if(!SplineMeshMaterial)
	{
		UE_LOG(LogActorComponent, Warning, TEXT("%s: spline mesh material not set"), *GetFullName())
	}
	if(!SM_Trajectory)
	{
		UE_LOG(LogActorComponent, Warning, TEXT("%s: static mesh for trajectory not set"), *GetFullName())
	}
}

#if WITH_EDITOR
void UOrbitComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
	
	constexpr float WorldRadius = UMyGameInstance::EditorDefaultWorldRadiusUU;
	constexpr float Alpha = UMyGameInstance::EditorDefaultAlpha;
	const FVector VecF1 = UMyGameInstance::EditorDefaultVecF1;
	
	const FName Name = PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName();

	static const FName FNameBTrajectoryShowSpline = GET_MEMBER_NAME_CHECKED(UOrbitComponent, bTrajectoryShowSpline);
	static const FName FNameSplineMeshLength = GET_MEMBER_NAME_CHECKED(UOrbitComponent, splineMeshLength);
	static const FName FNameVelocity = GET_MEMBER_NAME_CHECKED(UOrbitComponent, Velocity);
	static const FName FNameVelocityVCircle = GET_MEMBER_NAME_CHECKED(UOrbitComponent, VelocityVCircle);
	static const FName FNameVecVelocity = GET_MEMBER_NAME_CHECKED(UOrbitComponent, VecVelocity);
	static const FName FNameSMTrajectory = GET_MEMBER_NAME_CHECKED(UOrbitComponent, SM_Trajectory);
	static const FName FNameSplineMeshMaterial = GET_MEMBER_NAME_CHECKED(UOrbitComponent, SplineMeshMaterial);

	if(Name == FNameSplineMeshLength || Name == FNameBTrajectoryShowSpline)
	{
		UpdateWithParams(Alpha, WorldRadius, VecF1);
	}
	else if(Name == FNameVelocity || Name == FNameVelocityVCircle || Name == FNameVecVelocity)
	{
		const FVector VelocityNormal = VecVelocity.GetSafeNormal(1e-8, FVector(0., 1., 0.));
		const FVector VecRKepler = VecR - VecF1;

		if(Name == FNameVelocity)
		{
			VecVelocity = Velocity * VelocityNormal;
			VelocityVCircle = Velocity / sqrt(Alpha / VecRKepler.Length());
			UpdateWithParams(Alpha, WorldRadius, VecF1);
		}
		else if(Name == FNameVelocityVCircle)
		{
			Velocity = sqrt(Alpha / VecRKepler.Length()) * VelocityVCircle;
			VecVelocity = Velocity * VelocityNormal;
			UpdateWithParams(Alpha, WorldRadius, VecF1);
		}
		else if(Name == FNameVecVelocity)
		{
			Velocity = VecVelocity.Length();
			VelocityVCircle = Velocity / GetCircleVelocity(Alpha, VecF1);
			UpdateWithParams(Alpha, WorldRadius, VecF1);
		}
	}
	else if(Name == FNameSMTrajectory || Name == FNameSplineMeshMaterial)
	{
		UpdateWithParams(Alpha, WorldRadius, VecF1);
	}
}
#endif
