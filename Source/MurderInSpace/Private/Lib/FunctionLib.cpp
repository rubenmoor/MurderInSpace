#include "Lib/FunctionLib.h"

/*
 * calculate velocity vector given eccentricity vector, R, H
 * H is only used for its direction
 * when H = 0, the default velocity is returned
 */
FVector UFunctionLib::VecVelocity(FVector E, FVector R, FVector VecH, double Alpha, FVector Default)
{
    const FVector RNorm = R.GetSafeNormal();
    const FVector VecHNorm = VecH.GetSafeNormal();
    if(RNorm.IsZero() || VecHNorm.IsZero())
    {
        return Default;
    }
    FVector VecVelocity = VecHNorm.Cross(E + RNorm) * sqrt(Alpha / R.Length() / (RNorm.Dot(E) + 1));
    VecVelocity.Z = 0.;
    return VecVelocity;
}

double UFunctionLib::ScalarVelocitySquared(double R, double A, double Alpha)
{
    const double AInv = A == 0. ? 0. : 1. / A;
    return Alpha * (2. / R - AInv);
}

double UFunctionLib::VelocityInfinity(double E)
{
    return E > 0. ? sqrt(2 * E) : 0.;
}

/**
 * @brief eccentricity of elliptic Kepler orbit
 * @param R location vector
 * @param V velocity vector
 * @param Alpha gravitational parameter
 * @return E
 */
FVector UFunctionLib::Eccentricity(FVector R, FVector V, double Alpha)
{
    return ((V.SquaredLength() - Alpha / R.Length()) * R - R.Dot(V) * V) / Alpha;
}

/**
 * @brief second focus point of elliptic Kepler orbit
 * @param A semi-major axis
 * @param E eccentricity vector
 * @return F2
 */
FVector UFunctionLib::FocusPoint2(double A, FVector E)
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
double UFunctionLib::SemiMajorAxis(FVector VecR, FVector VecV, double Alpha)
{
    return 1. / (2. / VecR.Length() - VecV.SquaredLength() / Alpha);
}

/**
 * @brief Semi-minor axis
 * @param A 
 * @param ESquared 
 * @return B
 */
double UFunctionLib::SemiMinorAxis(double A, double ESquared)
{
    return A * sqrt(1 - ESquared);
}

/**
 * @brief Period of elliptic kepler orbit in seconds
 * @param a semi-major axis
 * @param Alpha gravitational parameter
 * @return T/s
 */
double UFunctionLib::PeriodEllipse(double a, double Alpha)
{
    return 2 * PI * sqrt(pow(a, 3) / Alpha);
}

/**
 * @brief Ramanujan's approximation for the perimeter of an ellipsis, error of order h^5
 * @param A 
 * @param B 
 * @return Perimeter
 */
double UFunctionLib::Perimeter(double A, double B)
{
    const auto H = pow(A - B, 2) / pow(A + B, 2);
    return PI * (A + B) * (1 + 3 * H / (10 + sqrt(4 - 3 * H)));
}

double UFunctionLib::AxialTidalForce(double R, double L, double M, double Alpha)
{
    return Alpha * L * M / (4 * pow(R, 3));
}
