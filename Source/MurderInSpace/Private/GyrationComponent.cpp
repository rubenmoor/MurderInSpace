// Fill out your copyright notice in the Description page of Project Settings.


#include "GyrationComponent.h"

#include "MyGameInstance.h"
#include "MyGameState.h"

UGyrationComponent::UGyrationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGyrationComponent::SetBody(UPrimitiveComponent* InBody)
{
	Body = InBody;
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
	}

	// TODO: meshes connected with sockets
	VecInertia = Body->GetInertiaTensor();
	
	if(VecL.IsZero())
	{
		TObjectPtr<UMyGameInstance> GI = GetWorld()->GetGameInstance<UMyGameInstance>();
		TObjectPtr<AMyGameState> GS = GetWorld()->GetGameState<AMyGameState>();
		if(!GS)
		{
			UE_LOG(LogActorComponent, Error, TEXT("%s: GameState null"), *GetFullName())
			return;
		}
		const float LRandom = GS->GetInitialAngularVelocity();
		UE_LOG(LogActorComponent, Warning, TEXT("Initializing with L = %f"), LRandom)
		VecL = GI->Random.VRand() * LRandom * VecInertia.Length();
		UE_LOG
		    ( LogActorComponent
		    , Display
		    , TEXT("%s: initializing angular momentum random: %f | %f | %f")
			, *GetFullName()
			, VecL.X
			, VecL.Y
			, VecL.Z
			)
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
