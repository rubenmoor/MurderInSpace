// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/GyrationComponent.h"

#include "Actors/IHasMesh.h"
#include "Modes/MyState.h"
#include "Net/UnrealNetwork.h"

UGyrationComponent::UGyrationComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = true;
}

void UGyrationComponent::FreezeState()
{
	if(!GetOwner<IHasMesh>())
	{
		UE_LOG(LogTemp, Error, TEXT("GetOwner<IHasMesh>(): %s"), *GetOwner()->GetFullName())
	}
	else
	RP_GyrationState = { GetOwner<IHasMesh>()->GetMesh()->GetComponentRotation(), VecL };
}

void UGyrationComponent::BeginPlay()
{
	Super::BeginPlay();

	// TODO: meshes connected with sockets
	VecInertia = GetOwner<IHasMesh>()->GetMesh()->GetInertiaTensor();

	if(GetOwnerRole() == ROLE_Authority)
	{
		if(VecL.IsZero())
		{
			UMyState* MyState = GEngine->GetEngineSubsystem<UMyState>();
			const FRnd Rnd = MyState->GetRndAny(this);
			const float LRandom = MyState->GetInitialAngularVelocity(Rnd);
			VecL = Rnd.Stream.VRand() * LRandom * VecInertia.Length();
			UE_LOG
				( LogMyGame
				, Display
				, TEXT("%s: Initializing angular momentum: (%.0f, %.0f, %.0f)")
				, *GetFullName()
				, VecL.X
				, VecL.Y
				, VecL.Z
				)
		}
		else
		{
			UE_LOG
				( LogMyGame
				, Error
				, TEXT("%s: Angular momentum set already: (%.0f, %.0f, %.0f), Owner role: %d")
				, *GetFullName()
				, VecL.X
				, VecL.Y
				, VecL.Z
				, GetOwnerRole()
				)
		}
	}
}

void UGyrationComponent::OnRep_GyrationState()
{
	GetOwner<IHasMesh>()->GetMesh()->SetWorldRotation(RP_GyrationState.Rot);
	VecL = RP_GyrationState.VecL;
}

// Called every frame
void UGyrationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UPrimitiveComponent* Body = GetOwner<IHasMesh>()->GetMesh();
	const FMatrix MatROld = FRotationMatrix(Body->GetComponentRotation());
	const FMatrix MatInertiaReverse = FMatrix
			( FVector(1./ VecInertia.X, 0, 0)
			, FVector(0, 1./ VecInertia.Y, 0)
			, FVector(0, 0, 1./ VecInertia.Z)
			, FVector(0, 0, 0)
			);
	VecOmega = (MatROld * MatInertiaReverse * MatROld.GetTransposed()).TransformFVector4(VecL);
	if(VecOmega.IsNearlyZero())
	{
		if(VecOmega.IsZero())
		{
			UE_LOG(LogMyGame, Error, TEXT("%s: VecOmega zero, skipping."), *GetFullName())
			return;
		}
		UE_LOG(LogMyGame, Warning, TEXT("%s: VecOmega nearly zero, just a warning."), *GetFullName())
	}
	const float Theta = VecOmega.Length();
	const FMatrix BNorm = FMatrix
			( FVector(0., -VecOmega.Z, VecOmega.Y)
			, FVector(VecOmega.Z, 0., -VecOmega.X)
			, FVector(-VecOmega.Y, VecOmega.X, 0.)
			, FVector(0, 0, 0)
			) * (1. / Theta);
	const FMatrix MatExp = FMatrix::Identity + BNorm * sin(Theta * DeltaTime) + BNorm * BNorm * (1. - cos(Theta * DeltaTime));
	const FMatrix MatRNew = MatExp * MatROld;
	
	Body->SetWorldRotation(MatRNew.ToQuat());
	L = VecL.Length();
	E = VecOmega.Dot(VecL) * 0.5;
}

#if WITH_EDITOR
void UGyrationComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
	
	const FName Name = PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName();

	static const FName FNameL = GET_MEMBER_NAME_CHECKED(UGyrationComponent, L);
	static const FName FNameE = GET_MEMBER_NAME_CHECKED(UGyrationComponent, E);

	if(Name == FNameL)
	{
		VecL *= L / VecL.Length();
	}
	else if(Name == FNameE)
	{
		VecL *= sqrt(2 * E * VecInertia.Length()) / L;
	}
}
#endif

void UGyrationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(UGyrationComponent, RP_GyrationState, COND_InitialOnly)
}
