// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/GyrationComponent.h"

#include "Lib/UStateLib.h"
#include "Net/UnrealNetwork.h"

UGyrationComponent::UGyrationComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UGyrationComponent::SetBody(UPrimitiveComponent* InBody)
{
	Body = InBody;
	UActorComponent::SetComponentTickEnabled(true);
}

void UGyrationComponent::FreezeState()
{
	if(IsValid(Body))
	{
		RP_GyrationState = { Body->GetComponentRotation(), VecL };
	}
}

void UGyrationComponent::BeginPlay()
{
	Super::BeginPlay();

	if(!Body)
	{
		UE_LOG(LogActorComponent, Display, TEXT("%s: body null, gyration disabled"), *GetFullName())
		return;
	}

	// TODO: meshes connected with sockets
	VecInertia = Body->GetInertiaTensor();

	if(GetOwnerRole() == ROLE_Authority)
	{
		if(VecL.IsZero())
		{
			const FRnd Rnd = UStateLib::GetRndUnsafe(this);
			const float LRandom = UStateLib::GetInitialAngularVelocity(Rnd);
			VecL = Rnd.Stream.VRand() * LRandom * VecInertia.Length();
			UE_LOG
				( LogActor
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
				( LogActorComponent
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
	Body->SetWorldRotation(RP_GyrationState.Rot);
	VecL = RP_GyrationState.VecL;
}

// Called every frame
void UGyrationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const FMatrix MatROld = FRotationMatrix(Body->GetComponentRotation());
	const FMatrix MatInertiaReverse = FMatrix
			( FVector(1./ VecInertia.X, 0, 0)
			, FVector(0, 1./ VecInertia.Y, 0)
			, FVector(0, 0, 1./ VecInertia.Z)
			, FVector(0, 0, 0)
			);
	VecOmega = (MatROld * MatInertiaReverse * MatROld.GetTransposed()).TransformFVector4(VecL);
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
	//static const FName FNameVecL = GET_MEMBER_NAME_CHECKED(UGyrationComponent, VecL);
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
