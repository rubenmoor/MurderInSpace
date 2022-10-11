// Fill out your copyright notice in the Description page of Project Settings.


#include "Lib/FunctionLib.h"

/**
 * @brief eccentricity of elliptic Kepler orbit
 * @param R location vector
 * @param V velocity vector
 * @param Alpha gravitational parameter
 * @return E
 */
FVector UFunctionLib::Eccentricity(FVector R, FVector V, float Alpha)
{
	   return (V.SquaredLength() / Alpha - 1.0 / R.Length()) * R - R.Dot(V) / Alpha * V;
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
 * @brief semi-major axis of elliptic orbit or hyperbolic trajectory
 *        for the elliptic orbit, a > 0; hyperbolic trajectory => a < 0
 * @param R vector of current location 
 * @param V vector of current velocity
 * @param Alpha gravitational parameter
 * @return a
 */
float UFunctionLib::SemiMajorAxis(FVector VecR, FVector VecV, float Alpha)
{
	   return 1. / (2. / VecR.Length() - VecV.SquaredLength() / Alpha);
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
 * @param Alpha gravitational parameter
 * @return T/s
 */
float UFunctionLib::PeriodEllipse(float a, float Alpha)
{
	   return 2 * PI * sqrt(pow(a, 3) / Alpha);
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

float UFunctionLib::AxialTidalForce(float R, float L, float M, float Alpha)
{
	   return Alpha * L * M / (4 * pow(R, 3));
}
