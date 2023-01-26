// Fill out your copyright notice in the Description page of Project Settings.


#include "MyComponents/MyCollisionComponent.h"

#include "Actors/IHasMesh.h"
#include "Actors/Orbit.h"
#include "Modes/MyState.h"
#include "Modes/MyGameInstance.h"
#include "Modes/MyGameState.h"

UMyCollisionComponent::UMyCollisionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMyCollisionComponent::HandleHit(const FHitResult& HitResult)
{
	// Normal is based on the object that was swept, 'ImpactNormal' is based on the object that was hit
	// Still, both are the same unless the object that was hit has a collision shape that isn't sphere or plane
	// In case two non-capsule shapes collide, I don't know what the value of 'Normal' would be

	auto* Other = HitResult.GetActor();
	auto* Orbit1 = GetOwner<IHasOrbit>()->GetOrbit();
	auto* Orbit2 = Cast<IHasOrbit>(Other)->GetOrbit();
	const FVector VecV1 = Orbit1->GetVecVelocity();
	const FVector VecV2 = Orbit2->GetVecVelocity();
	// TODO: mass of asteroids
	// 'GetMass' requires simulate physics
	const double M1 = pow(GetOwner<IHasMesh>()->GetMesh()->GetLocalBounds().SphereRadius, 3);
	const double M2 = pow(Cast<IHasMesh>(HitResult.GetActor())->GetMesh()->GetLocalBounds().SphereRadius, 3);
	// velocity of the center of mass
	const FVector VecVCoM = (M1 * VecV1 + M2 * VecV2) / (M1 + M2);
	// the idea is to subtract the CoM velocity to make sure that u1 and u2 are on plane with the normal,
	// I am not sure about that, however
	const FVector VecU1 = VecV1 - VecVCoM;
	const FVector VecU2 = VecV2 - VecVCoM;

	// new base vectors: VecN, VecO
	const FVector VecN = HitResult.Normal;
	// U1 = Alpha1 * VecN + Beta1 * VecO
	const double Alpha1 = VecU1.Dot(VecN);
	const FVector VecU1O = VecU1 - Alpha1 * VecN;
	const double Alpha2 = VecU2.Dot(VecN);
	const FVector VecU2O = VecU2 - Alpha2 * VecN;

	const double UBar = (M1 * Alpha1 + M2 * Alpha2) / (M1 + M2);
	double K = 0.5;
	// partially elastic collision, k in [0, 1] where k = 0 is plastic and k = 1 elastic collision, respectively
	const FVector VecW1 = (UBar - M2 * (Alpha1 - Alpha2) / (M1 + M2) * K) * VecN + VecU1O + VecVCoM;
	const FVector VecW2 = (UBar - M1 * (Alpha2 - Alpha1) / (M1 + M2) * K) * VecN + VecU2O + VecVCoM;

	auto* MyState = GEngine->GetEngineSubsystem<UMyState>();
	auto* GI = GetWorld()->GetGameInstance<UMyGameInstance>();
	auto* GS = GetWorld()->GetGameState<AMyGameState>();
	FPhysics Physics = MyState->GetPhysics(GS);
	FInstanceUI InstanceUI = MyState->GetInstanceUI(GI);
	Orbit1->Update(VecW1 - VecV1, Physics, InstanceUI);
	Orbit2->Update(VecW2 - VecV2, Physics, InstanceUI);
}
