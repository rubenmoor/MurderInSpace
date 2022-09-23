// Fill out your copyright notice in the Description page of Project Settings.


#include "GyrationComponent.h"

#include "MyGameInstance.h"

UGyrationComponent::UGyrationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UGyrationComponent::BeginPlay()
{
	Super::BeginPlay();

	if(!Body)
	{
		Body = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
		if(!Body)
		{
			Body = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
		}
		if(!Body)
		{
			UE_LOG(LogActorComponent, Error, TEXT("%s: body null"), *GetFullName())
			return;
		}
		UE_LOG(LogActorComponent, Warning,
			TEXT("%s: BeginPlay: defaulting body to %s"), *GetFullName(), *Body->GetFullName())
	}

	if(VecInertia.IsZero())
	{
		VecInertia = Body->GetInertiaTensor();
		UE_LOG
			( LogActorComponent
			, Display
			, TEXT("%s: initializing inertia Vector: %f | %f | %f")
			, *GetFullName()
			, VecInertia.X
			, VecInertia.Y
			, VecInertia.Z
			)
	}

	TObjectPtr<UMyGameInstance> GI = GetWorld()->GetGameInstance<UMyGameInstance>();
	if(VecL.IsZero())
	{
		VecL = GI->Random.VRand() * GI->Random.RandRange(.01, .1) * VecInertia.Length();
	}
}

void UGyrationComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
	
	const FName Name = PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName();

	static const FName FNameL = GET_MEMBER_NAME_CHECKED(UGyrationComponent, L);
	static const FName FNameVecL = GET_MEMBER_NAME_CHECKED(UGyrationComponent, VecL);
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
	const FVector VecOmega = (MatROld * MatInertiaReverse * MatROld.GetTransposed()).TransformFVector4(VecL);
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

