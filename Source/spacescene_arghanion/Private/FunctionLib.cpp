// Fill out your copyright notice in the Description page of Project Settings.


#include "FunctionLib.h"

#include "Kismet/GameplayStatics.h"

/**
 * @brief eccentricity of elliptic Kepler orbit
 * @param R location vector
 * @param V velocity vector
 * @param MU gravitational parameter
 * @return E
 */
FVector UFunctionLib::Eccentricity(FVector R, FVector V, float MU)
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
 * @brief semi-major axis of elliptic orbit or hyperbolic trajectory
 *        for the elliptic orbit, a > 0; hyperbolic trajectory => a < 0
 * @param R vector of current location 
 * @param V vector of current velocity
 * @param MU gravitational parameter
 * @return a
 */
float UFunctionLib::SemiMajorAxis(FVector VecR, FVector VecV, float MU)
{
	return 1. / (2. / VecR.Length() - VecV.SquaredLength() / MU);
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
 * @param MU gravitational parameter
 * @return T/s
 */
float UFunctionLib::PeriodEllipse(float a, float MU)
{
	return 2 * PI * sqrt(pow(a, 3) / MU);
}

/**
 * @brief Orbital velocity given the semi-major axis and current distance to F1
 * @param R distance to focal point 1
 * @param A semi-major axis
 * @param MU gravitational parameter
 * @return v
 */
float UFunctionLib::VelocityEllipse(float R, float A, float MU)
{
    return std::max(sqrt(MU * (2.0 / R - 1.0 / A)), 1.);
}

float UFunctionLib::VelocityParabola(float R, float MU)
{
    return sqrt(MU * 2.0 / R);
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

TObjectPtr<AGameModeAsteroids> UFunctionLib::GetGameModeAsteroids(const UObject* WorldContextObject)
{
    const auto GameMode = StaticCast<AGameModeAsteroids*>(WorldContextObject->GetWorld()->GetAuthGameMode());
    if(!GameMode)
    {
        UE_LOG(LogTemp, Error, TEXT("Game mode must implement `AGameModeAsteroids`"));
        GEngine->AddOnScreenDebugMessage(0, 3, FColor::Red, TEXT("Critical error: no game mode"));
    }
    return GameMode;
}

void UFunctionLib::LogVector(FLogCategoryName CategoryName, ELogVerbosity::Type Verbosity, FVector Vector)
{
    //UE_LOG(CategoryName, Verbosity, TEXT("%s: (%f, %f, %f)"));
}

