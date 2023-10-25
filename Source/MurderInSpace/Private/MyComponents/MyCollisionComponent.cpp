#include "MyComponents/MyCollisionComponent.h"

#include "GameplayAbilitySystem/GA_Embrace.h"
#include "Spacebodies/IHasMesh.h"
#include "Logging/StructuredLog.h"
#include "Orbit/Orbit.h"
#include "Modes/MyGameInstance.h"
#include "Modes/MyGameState.h"

UMyCollisionComponent::UMyCollisionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMyCollisionComponent::HandleHit(FHitResult& HitResult, UPrimitiveComponent* PrimitiveComponent) const
{
	auto* Other = HitResult.GetActor();
	if(!IsValid(Other))
	{
		UE_LOGFMT(LogMyGame, Error, "{0}: Other actor invalid"
			, GetFName());
		return;
	}

	// if(Other->GetName() > GetOwner()->GetName())
	// {
	// 	// TODO: too many hits are being ignored. have to distinguish between legitimately ignored hits and errors
	// 	//UE_LOG(LogMyGame, Display, TEXT("%s: Ignoring blocking hit with %s")
	// 	//	, *GetFName()
	// 	//	, *Other->GetName()
	// 	//	)
	// 	return;
	// }
	
	if(HitResult.bStartPenetrating)
	{
		//UE_LOG(LogMyGame, Warning, TEXT("%s: bStartPenetrating, hit due to rotation")
		//	, *GetFName()
		//	)
		// TODO: deal with hit results due to rotation here
		const FVector OldR = PrimitiveComponent->GetComponentLocation();
		PrimitiveComponent->SetWorldLocation(OldR + HitResult.Normal * HitResult.PenetrationDepth * 1.1);
		PrimitiveComponent->SetWorldLocation(OldR, true, &HitResult);
		if(!HitResult.bBlockingHit)
		{
			UE_LOGFMT(LogMyGame, Error, "{0}: bStartPenetrating, no hit after correction", GetFName());
			return;
		}
		else if(HitResult.bStartPenetrating)
		{
			UE_LOGFMT(LogMyGame, Error, "{0}: bStartPenetrating, start penetrating after correction", GetFName());
			return;
		}
	}
	// Normal is based on the object that was swept, 'ImpactNormal' is based on the object that was hit
	// Still, both are the same unless the object that was hit has a collision shape that isn't sphere or plane
	// In case two non-capsule shapes collide, I don't know what the value of 'Normal' would be

	// UE_LOG(LogMyGame, Warning, TEXT("%s: Blocking hit with %s")
	// 	, *GetFName()
	// 	, *Other->GetName()
	// 	)
	
	auto [EmbraceActor, MyCombinedMass] = GetEmbraceActor(GetOwner());
	auto [OtherEmbraceActor, OtherMass] = GetEmbraceActor(Other);

	auto* Orbit1 = GetOwner<IHasOrbit>()->GetOrbit();
	auto Orbit1Embrace = nullptr; // EmbraceActor ? TOptional(Cast<IHasOrbit>(EmbraceActor)->GetOrbit()) : TOptional<IHasOrbit>();
	auto* Orbit2 = Cast<IHasOrbit>(Other)->GetOrbit();
	auto Orbit2Embrace = nullptr; // OtherEmbraceActor ? TOptional(Cast<IHasOrbit>(OtherEmbraceActor)->GetOrbit()) : TOptional<IHasOrbit>();
	
	const FVector VecV1 = Orbit1->GetVecVelocity();
	const FVector VecV2 = Orbit2->GetVecVelocity();

	FVector VecN = HitResult.ImpactNormal;

	switch(MyCollisionDimensions)
	{
	using enum EMyCollisionDimensions;
	case CollisionXYPlane:
		VecN = VecN.GetUnsafeNormal2D();
		break;
	case CollisionXYZ:
		// leave the impact normal
		break;
	}

	// collision restitution: combination by multiplication
	double K = CoR * Cast<IHasCollision>(Other)->GetCollisionComponent()->CoR;

	// partially elastic collision, k in [0, 1] where k = 0 is plastic and k = 1 elastic collision, respectively
	
	// calculation for 3 dimensions
	// https://en.wikipedia.org/wiki/Inelastic_collision
 	const double J = MyCombinedMass * OtherMass / (MyCombinedMass + OtherMass) * (1. + K) * (VecV2 - VecV1).Dot(VecN);
	const FVector VecDeltaV1 = J / MyCombinedMass * VecN;
	const FVector VecDeltaV2 = -J / OtherMass * VecN;

	switch(MyCollisionDimensions)
	{
	using enum EMyCollisionDimensions;
	case CollisionXYPlane:
		// TODO: z must be 0
		break;
	case CollisionXYZ:
		// leave the impact normal
		break;
	}

	const auto* GS = GetWorld()->GetGameState<AMyGameState>();
	Orbit1->Update(VecDeltaV1, GS->RP_Physics);
	Orbit2->Update(VecDeltaV2, GS->RP_Physics);
}

double UMyCollisionComponent::GetMyMass()
{
	checkf(bMassInitialized || bOverrideMass, TEXT("%s: mass must be initialized, or overridden"), *GetFName().ToString())
	const double Mass = bOverrideMass ? MassOverride : MyMass;
	check(Mass != 0.)
	return Mass;
}

#if WITH_EDITOR
void UMyCollisionComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

    const FName Name = PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName();

    static const FName FMassOverride = GET_MEMBER_NAME_CHECKED(UMyCollisionComponent, MassOverride);
    static const FName FDensity      = GET_MEMBER_NAME_CHECKED(UMyCollisionComponent, Density);
	static const FName FDensityExponent = GET_MEMBER_NAME_CHECKED(UMyCollisionComponent, DensityExponent);

    if(Name == FMassOverride)
    {
		bOverrideMass = true;
    }
    else if(Name == FDensity || Name == FDensityExponent)
    {
		UpdateMass(GetOwner<IHasMesh>()->GetBounds().SphereRadius);
    }
}
#endif

// TODO: TOptional<AActor> doesn't work, use TOptional<AActor*> instead
TTuple<TOptional<AActor>, double> UMyCollisionComponent::GetEmbraceActor(AActor* InActor)
{
	auto EmbraceActor = TOptional<AActor>();
	double CombinedMass = Cast<IHasCollision>(InActor)->GetCollisionComponent()->GetMyMass();
	if(InActor->Implements<UCanEmbrace>())
		EmbraceActor = *Cast<ICanEmbrace>(InActor)->GetEmbracedActor();
	
	if(InActor->Implements<UCanBeEmbraced>())
	{
		check(!EmbraceActor.IsSet())
		EmbraceActor = *Cast<ICanBeEmbraced>(InActor)->GetEmbracingActor();
	}
	CombinedMass += EmbraceActor.IsSet() ? Cast<IHasCollision>(&*EmbraceActor)->GetCollisionComponent()->GetMyMass() : 0.;
	return TTuple<TOptional<AActor>, double>{TOptional(*EmbraceActor), CombinedMass};
}

void UMyCollisionComponent::UpdateMass(double Radius)
{
	MyMass = Density * pow(Radius, DensityExponent) * FPhysics::MassScaleFactor;
	checkf(MyMass != 0., TEXT("%s: mass zero"), *GetFName().ToString())
	bMassInitialized = true;
}
