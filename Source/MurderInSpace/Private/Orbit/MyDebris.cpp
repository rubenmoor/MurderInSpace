#include "Orbit/MyDebris.h"

#include "Actors/Blackhole.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Modes/MyGameState.h"
#include "Modes/MyState.h"

AMyDebris::AMyDebris()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	bNetLoadOnClient = false;
	bReplicates = false;

	ISMComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>("ISMComponent");
	SetRootComponent(ISMComponent);
}

void AMyDebris::BeginPlay()
{
	Super::BeginPlay();

	const auto MyState = UMyState::Get();
	const auto GS = GetWorld()->GetGameState<AMyGameState>();
	const auto Physics = MyState->GetPhysics(GS);
	
	FRandomStream RandomStream;
	IndividualVelocities.SetNum(Num);
	TArray<FTransform> Transforms;
	Transforms.SetNum(Num);
	Scales.SetNum(Num);

	for(int32 i = 0; i < Num; i++)
	{
		double R = RandomStream.FRand() * Radius;
		FVector2D ThetaPhi = FVector2D(RandomStream.FRand() * PI, RandomStream.FRand() * 2 * PI - PI);
		Transforms[i].SetLocation(R * ThetaPhi.SphericalToUnitCartesian());
		Transforms[i].SetRotation(FQuat::MakeFromRotator(UKismetMathLibrary::RandomRotator(true)));
		double Scale = (RandomStream.FRand() + .5) * AverageScale;
		Scales[i] = Scale;
		Transforms[i].SetScale3D(Scale * ScaleFactor * FVector::One());
		IndividualVelocities[i] = VecCoMVelocity + Transforms[i].GetLocation() - InitialAttenuation * IndividualVelocities[i];
	}
	ISMComponent->AddInstances(Transforms, false);
	UE_LOG(LogMyGame, Warning, TEXT("%s: Debris, Num: %d, average scale: %f"), *GetFullName(), Num, AverageScale)
}

void AMyDebris::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	Age += DeltaSeconds;
	if(Age >= Lifetime)
	{
		Destroy();
		return;
	}

	const auto MyState = UMyState::Get();
	const auto GS = GetWorld()->GetGameState<AMyGameState>();
	const auto Physics = MyState->GetPhysics(GS);
	const auto Blackhole = GS->GetBlackhole();
	
	for(int32 i = 0; i < ISMComponent->GetInstanceCount(); ++i)
	{
		FTransform Transform;
		ISMComponent->GetInstanceTransform(i, Transform);
		auto Loc = Transform.GetLocation();
		auto VecR = Loc + GetActorLocation();
		auto VecRKepler = VecR - Physics.VecF1;
		
		// attenuate to have the debris fall into the black hole
		IndividualVelocities[i] *= 1. - TraversalAttenuation * DeltaSeconds; // * ToTangential(IndividualVelocities[i], VecRKepler);
		
		IndividualVelocities[i] += 2. * Physics.Alpha / VecRKepler.SquaredLength() * -VecRKepler.GetUnsafeNormal() * DeltaSeconds;
		
		auto VecNewRKepler = VecRKepler + IndividualVelocities[i] * DeltaSeconds;
		
		if(VecNewRKepler.Length() < Blackhole->GetKillRadius())
		//if(false)
		{
			UE_LOG(LogMyGame, Warning, TEXT("%s: VecNewRKepler: (%.1f, %.1f, %.1f), Length: %f"), *GetFullName(), VecNewRKepler.X, VecNewRKepler.Y, VecNewRKepler.Z, VecNewRKepler.Length())
			ISMComponent->RemoveInstance(i);
			--i;
		}
		else
		{
			Transform.SetLocation(VecNewRKepler - GetActorLocation());
			Transform.SetScale3D(Scales[i] * (1. - Age / Lifetime) * FVector::One());
			ISMComponent->UpdateInstanceTransform(i, Transform, false, false, true);
		}
		
	}
	//ISMComponent->MarkRenderInstancesDirty();
	ISMComponent->MarkRenderStateDirty();
}

FVector AMyDebris::ToTangential(FVector InVec, FVector InVecRKepler)
{
	const FVector RNorm = InVecRKepler.GetUnsafeNormal();
	return InVec - InVec.Dot(RNorm) * RNorm;
}

