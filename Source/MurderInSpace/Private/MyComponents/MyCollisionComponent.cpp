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

void UMyCollisionComponent::HandleHit(FHitResult& HitResult)
{
	if(HitResult.bStartPenetrating)
	{
		// TODO: deal with hit results due to rotation here
		UE_LOG(LogMyGame, Warning, TEXT("%s: bStartPenetrating, correcting position and hitting again"), *GetFullName())
		auto* PrimitiveComponent = Cast<IHasMesh>(GetOwner())->GetMesh();
		PrimitiveComponent->SetRelativeLocation(-HitResult.Normal * HitResult.PenetrationDepth * 1.1);
		PrimitiveComponent->SetRelativeLocation(FVector::Zero(), true, &HitResult);
		// I don't know why, yet: treating the bStartPenetrating case like a hit, doesn't seem to work
		if(!HitResult.bBlockingHit)
		{
			UE_LOG(LogMyGame, Error, TEXT("%s: bStartPenetrating, no hit after correction"), *GetFullName())
			return;
		}
		else if(HitResult.bStartPenetrating)
		{
			UE_LOG(LogMyGame, Error, TEXT("%s: bStartPenetrating, start penetrating after correction"), *GetFullName())
			return;
		}
	}
	// Normal is based on the object that was swept, 'ImpactNormal' is based on the object that was hit
	// Still, both are the same unless the object that was hit has a collision shape that isn't sphere or plane
	// In case two non-capsule shapes collide, I don't know what the value of 'Normal' would be

	auto* Other = HitResult.GetActor();
	auto* Orbit1 = GetOwner<IHasOrbit>()->GetOrbit();
	auto* Orbit2 = Cast<IHasOrbit>(Other)->GetOrbit();
	const FVector VecV1 = Orbit1->GetVecVelocity();
	const FVector VecV2 = Orbit2->GetVecVelocity();
	// TODO: use PhysicsMaterial for mass density
	const double M1 = GetOwner<IHasMesh>()->GetMyMass();
	const double M2 = Cast<IHasMesh>(HitResult.GetActor())->GetMyMass();
	// the idea is to subtract the CoM velocity to make sure that u1 and u2 are on plane with the normal,
	// I am not sure about that, however
	// new base vectors: VecN, VecO
	const FVector VecN = HitResult.Normal;
	// U1 = Alpha1 * VecN + Beta1 * VecO
	const double Alpha1 = VecV1.Dot(VecN);
	const FVector VecU1O = VecV1 - Alpha1 * VecN;
	const double Alpha2 = VecV2.Dot(VecN);
	const FVector VecU2O = VecV2 - Alpha2 * VecN;

	const double UBar = (M1 * Alpha1 + M2 * Alpha2) / (M1 + M2);
	// TODO: use PhysicsMaterial for k value, e.g. k = std::min(1., k1 + k2)
	//double K = 0.5;
	double K = 1.;
	// partially elastic collision, k in [0, 1] where k = 0 is plastic and k = 1 elastic collision, respectively
	const FVector VecW1 = (UBar - M2 * (Alpha1 - Alpha2) / (M1 + M2) * K) * VecN + VecU1O;
	const FVector VecW2 = (UBar - M1 * (Alpha2 - Alpha1) / (M1 + M2) * K) * VecN + VecU2O;

	auto* MyState = GEngine->GetEngineSubsystem<UMyState>();
	const auto* GI = GetWorld()->GetGameInstance<UMyGameInstance>();
	const auto* GS = GetWorld()->GetGameState<AMyGameState>();
	const FPhysics Physics = MyState->GetPhysics(GS);
	const FInstanceUI InstanceUI = MyState->GetInstanceUI(GI);
	Orbit1->Update(VecW1 - VecV1, Physics, InstanceUI);
	Orbit2->Update(VecW2 - VecV2, Physics, InstanceUI);
}
