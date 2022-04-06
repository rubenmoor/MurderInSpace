// Fill out your copyright notice in the Description page of Project Settings.


#include "FunctionLib.h"

/**
 * @brief get the Kepler orbit: circle, ellipse, parabola, or hyperbola
 * @param VecR location vector
 * @param VecV velocity
 * @return orbit information
 */
Orbit UFunctionLib::GetOrbit(const FVector VecR, const FVector VecV)
{
	constexpr auto Tolerance = 1E-8;
	const auto VecH = VecR.Cross(VecV);
	const float P = VecH.SquaredLength() / MU;
	const auto VecE = Eccentricity(VecR, VecV);
	const auto E = VecE.Length();
	const auto VecENorm = VecE.GetSafeNormal(Tolerance);
	if(VecENorm.IsZero())
	{
		const float R = VecR.Length();
		return FCircle { R, VecH, Period(R)};
	}
	else if(abs(E - 1) < Tolerance)
	{
		return FParabola { P, VecE, VecH };
	}
	else if(E > 1)
	{
		const float A = P / (VecE.SquaredLength() - 1);
		const float B = P / sqrt(VecE.SquaredLength() - 1);
		return FHyperbola { A, B, VecE, VecH };
	}
	else // 0 < e < 1
	{
		const float A = P / (1 - VecE.SquaredLength());
		return FEllipse { A, VecH, VecE, Period(A) };
	}
}

TArray<FSplinePoint> UFunctionLib::CreatePointsCircle(FVector VecR, FVector VecH)
{
	const auto HNorm = VecH.GetSafeNormal();
	const auto VecP2 = HNorm.Cross(VecR);
	const auto VecT1 = HNorm.Cross(VecR) * SplineToCircle;
	const auto VecT4 = VecR * SplineToCircle;
	return
	  { FSplinePoint(0,  VecR ,  VecT1,  VecT1)
		, FSplinePoint(1,  VecP2, -VecT4, -VecT4)
		, FSplinePoint(2, -VecR , -VecT1, -VecT1)
		, FSplinePoint(3, -VecP2,  VecT4,  VecT4)
		};
}

/**
 * @brief Create spline points of an elliptic orbit
 * @param A semi-major axis
 * @param VecE eccentricity vector
 * @param VecH specific angular momentum H = R x V
 * @return points
 */
TArray<FSplinePoint> UFunctionLib::CreatePointsEllipsis(float A, FVector VecE, FVector VecH)
{
	const auto VecENorm = VecE.GetUnsafeNormal();
	const auto B = A * sqrt(1 - VecE.SquaredLength());
	const auto Vertex1 = A * (1 - VecE.Length()) * VecENorm;
	const auto Vertex2 = A * (1 + VecE.Length()) * -VecENorm;
	const auto Orthogonal = VecH.Cross(VecE).GetSafeNormal();
	const auto Covertex1 =  B * Orthogonal - A * VecE;
	const auto Covertex2 = -B * Orthogonal - A * VecE;
	const auto T1 = Orthogonal * SplineToCircle * B;
	const auto T4 = VecENorm * SplineToCircle * A;
	return
	 { FSplinePoint(0, Vertex1  ,  T1,  T1)
		, FSplinePoint(1, Covertex1, -T4, -T4)
		, FSplinePoint(2, Vertex2  , -T1, -T1)
		, FSplinePoint(3, Covertex2,  T4,  T4)
		};
}

/**
 * @brief Create spline points of a parabola orbit
 * @param P = h^2 / MU, a constant of motion
 * @param VecE eccentricity vector
 * @param VecH specific angular momentum
 * @return points
 */
TArray<FSplinePoint> UFunctionLib::CreatePointsParabola(float P, FVector VecE, FVector VecH)
{
	const auto VecENorm = VecE.GetUnsafeNormal();
	const auto VecHNorm = VecH.GetUnsafeNormal();
	const auto VecHorizontal = VecHNorm.Cross(VecENorm);
	// TODO: check if tangents need setup
	TArray<FSplinePoint> Points = { FSplinePoint(0, VecENorm * P / 2 ) };
	auto parabola = [VecHorizontal, VecENorm, P](int n, int sign)
	{
		return (sign * n * VecHorizontal + VecENorm / 2.0 * ( 1 - pow(n, 2))) * P;
	};
	for(int i = 1; i < 10; i++)
	{
		Points.Push(FSplinePoint(2.0 * i - 1, parabola(i, 1)));
		Points.Push(FSplinePoint(2.0 * i    , parabola(i, -1)));
	}
	return Points;
}

/**
 * @brief 
 * @return 
 */
TArray<FSplinePoint> UFunctionLib::CreatePointsHyperbola()
{
	return TArray<FSplinePoint>();
}

/**
 * @brief eccentricity of elliptic Kepler orbit
 * @param R location vector
 * @param V velocity vector
 * @return E
 */
FVector UFunctionLib::Eccentricity(FVector R, FVector V)
{
	return (V.SquaredLength() / MU - 1.0 / R.Length()) * R - R.Dot(V) / MU * V;
}

/**
 * @brief second focus point of elliptic Kepler orbit
 * @param A semi-major axis
 * @param E eccentricity vector
 * @return F2
 */
FVector UFunctionLib::FocusPoint2(float A, FVector E)
{
	return  -2 * A * E;
}

/**
 * @brief semi-major axis of elliptic kepler orbit
 * @param R vector of current location 
 * @param V vector of current velocity
 * @return a
 */
float UFunctionLib::SemiMajorAxis(FVector R, FVector V)
{
	return MU * R.Length() / ( 2 * MU - R.Length() * V.SquaredLength());
}

/**
 * @brief Semi-minor axis
 * @param A 
 * @param ESquared 
 * @return B
 */
float UFunctionLib::SemiMinorAxis(float A, float ESquared)
{
	return A * sqrt(1 - ESquared);
}

/**
 * @brief Period of elliptic kepler orbit in seconds
 * @param a semi-major axis
 * @return T/s
 */
float UFunctionLib::Period(float a)
{
	return 2 * PI * sqrt(pow(a, 3) / MU);
}

/**
 * @brief Orbital velocity given the semi-major axis and current distance to F1
 * @param R 
 * @param A 
 * @return v
 */
float UFunctionLib::Velocity(float R, float A)
{
	const auto v = sqrt(MU * (2.0 / R - 1.0 / A));
	UE_LOG(LogTemp, Display, TEXT("UFunctionLib::Velocity: %f"), v );
	return v;
}

/**
 * @brief Ramanujan's approximation for the perimeter of an ellipsis, error of order h^5
 * @param A 
 * @param B 
 * @return Perimeter
 */
float UFunctionLib::Perimeter(float A, float B)
{
	const auto H = pow(A - B, 2) / pow(A + B, 2);
	return PI * (A + B) * (1 + 3 * H / (10 + sqrt(4 - 3 * H)));
}

