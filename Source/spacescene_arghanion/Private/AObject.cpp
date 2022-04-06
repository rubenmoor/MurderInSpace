#include "AObject.h"

#include "FunctionLib.h"
#include "VectorTypes.h"

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

AAObject::AAObject()
{
	PrimaryActorTick.bCanEverTick = true;

	// member variables

	// components

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(Root);

	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	Spline->SetupAttachment(Root);

	MeshRoot = CreateDefaultSubobject<USceneComponent>(TEXT("MeshRoot"));
	MeshRoot->SetupAttachment(Root);
	
	MainMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainMesh"));
	MainMesh->SetupAttachment(MeshRoot);

	// spline points initialization
	
	Spline->SetClosedLoop(true, false);
}

void AAObject::Tick(float DeltaTime)
{
	const auto DeltaR = std::visit(overloaded
	 	{ [&](const FCircle Circle)
	 		{
	 			return VelocityScalar * DeltaTime;
	 		}
	 	, [&](const FEllipse Ellipse)
	 		{
	 			const float R = MainMesh->GetComponentLocation().Length();
	 			VelocityScalar = UFunctionLib::Velocity(R, Ellipse.A);
	 			return VelocityScalar * DeltaTime;
	 		}
	 	, [&](const FParabola Parabola)
	 		{
	 			return VelocityScalar * DeltaTime;
	 		}
	 	, [&](const FHyperbola Hyperbola)
	 		{
	 			return VelocityScalar * DeltaTime;
	 		}
		}, MyOrbit);
	SplineDistance = fmod(SplineDistance + DeltaR, Spline->GetSplineLength());
	Velocity = Spline->GetTangentAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World).GetSafeNormal() * VelocityScalar;
	const auto NewLocation = Spline->GetLocationAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World);
	MeshRoot->SetRelativeLocation(NewLocation);
	Super::Tick(DeltaTime);
}

void AAObject::BeginPlay()
{
	Super::BeginPlay();
}

void AAObject::OnConstruction(const FTransform& Transform)
{
	const auto VecActorLocation = Transform.GetLocation().IsZero() ? FVector(0, 1000, 0) : Transform.GetLocation();
	const auto VecMeshRootLocation = MeshRoot->GetComponentLocation();
	auto VecLocation = VecMeshRootLocation.IsZero() ? VecActorLocation : VecMeshRootLocation;
	
	VecLocation.SetComponentForAxis(EAxis::Z, 0);
	
	this->SetActorLocation(FVector::Zero());
	Root->SetWorldLocation(FVector::Zero());
	MeshRoot->SetWorldLocation(VecLocation);

	const auto VecR = MeshRoot->GetComponentLocation();
	//const auto VecV = sqrt(UFunctionLib::MU / VecR.Length()) * FVector(0, 0, 1).Cross(VecR.GetSafeNormal());
	const auto VecV = 0.8 * sqrt(UFunctionLib::MU / VecR.Length()) * (FVector(.2, 0, 0) + FVector(0.1, 0.2, 0.9).Cross(VecR.GetSafeNormal()));
	Velocity = VecV;
	VelocityScalar = VecV.Length();
	MyOrbit = UFunctionLib::GetOrbit(VecR, VecV);
	const auto Points = std::visit(overloaded
		{ [&](const FCircle Circle)
			{
				OrbitInformation = FString::Printf(TEXT("Circle, R: %f, H: (%f, %f, %f), T: %f"), Circle.R, Circle.VecH.X, Circle.VecH.Y, Circle.VecH.Z, Circle.T);
				return UFunctionLib::CreatePointsCircle(VecR, Circle.VecH);
			}
		, [&](const FEllipse Ellipse)
			{
				OrbitInformation = FString::Printf(TEXT("Ellipse, A: %f, E: (%f, %f, %f), H: (%f, %f, %f), T: %f"), Ellipse.A, Ellipse.VecE.X, Ellipse.VecE.Y, Ellipse.VecE.Z, Ellipse.VecH.X, Ellipse.VecH.Y, Ellipse.VecH.Z, Ellipse.T);
				return UFunctionLib::CreatePointsEllipsis(Ellipse.A, Ellipse.VecE, Ellipse.VecH);
			}
		, [&](const FParabola Parabola)
			{
				OrbitInformation = FString::Printf(TEXT("Parabola, P: %f, E: (%f, %f, %f), H: (%f, %f, %f)"), Parabola.P, Parabola.VecE.X, Parabola.VecE.Y, Parabola.VecE.Z, Parabola.VecH.X, Parabola.VecH.Y, Parabola.VecH.Z);
				return UFunctionLib::CreatePointsParabola(Parabola.P, Parabola.VecE, Parabola.VecH);
			}
		, [&](const FHyperbola Hyperbola)
			{
				OrbitInformation = FString::Printf(TEXT("Hyperbola, A: %f, E: (%f, %f, %f), H: (%f, %f, %f)"), Hyperbola.A, Hyperbola.VecE.X, Hyperbola.VecE.Y, Hyperbola.VecE.Z, Hyperbola.VecH.X, Hyperbola.VecH.Y, Hyperbola.VecH.Z);
				return UFunctionLib::CreatePointsHyperbola();
			}
		}, MyOrbit);
	Spline->ClearSplinePoints(false);
	Spline->AddPoints(Points);
	Spline->UpdateSpline();

	// update current distance along spline
	SplineDistance = Spline->GetDistanceAlongSplineAtSplineInputKey(Spline->FindInputKeyClosestToWorldLocation(VecR));
}
