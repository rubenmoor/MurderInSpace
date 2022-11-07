#include "Actors/PawnInSpace.h"

#include "Modes/MyGameState.h"
#include "Net/UnrealNetwork.h"

APawnInSpace::APawnInSpace()
{
	PrimaryActorTick.bCanEverTick = false;

	// components

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	//Root->SetMobility(EComponentMobility::Stationary);
	SetRootComponent(Root);

	MovableRoot = CreateDefaultSubobject<USceneComponent>(TEXT("MovableRoot"));
	MovableRoot->SetupAttachment(Root);
	
	Orbit = CreateDefaultSubobject<UOrbitComponent>(TEXT("Orbit"));
	Orbit->SetupAttachment(Root);
	Orbit->SetMovableRoot(MovableRoot);
	
	SplineMeshParent = CreateDefaultSubobject<USceneComponent>(TEXT("SplineMesh"));
	SplineMeshParent->SetupAttachment(Orbit);
	SplineMeshParent->SetMobility(EComponentMobility::Stationary);
	Orbit->SetSplineMeshParent(SplineMeshParent);

	// for the editor
	Orbit->UpdateVisibility(UStateLib::GetPlayerUIEditorDefault());
	
	bNetLoadOnClient = false;
	AActor::SetReplicateMovement(false);
}

void APawnInSpace::UpdateLookTarget(FVector Target)
{
	// TODO
}

// void APawnInSpace::ClientRPC_UpdateOrbitStates_Implementation(const TArray<FOrbitState>& OrbitStates)
// {
// 	for(auto OrbitState : OrbitStates)
// 	{
// 		UOrbitComponent* SomeOrbit = Cast<UOrbitComponent>(StaticFindObjectFastSafe
// 			(UOrbitComponent::StaticClass()
// 			, GetWorld()
// 			, OrbitState.OrbitFName
// 			));
// 		if(!SomeOrbit)
// 		{
// 			UE_LOG
// 				(LogActor
// 				, Error
// 				, TEXT("%s: Could not find UOrbitComponent with FName %s.")
// 				, *GetFullName()
// 				, *OrbitState.OrbitFName.ToString()
// 				)
// 		}
// 		else
// 		{
// 			SomeOrbit->ApplyOrbitState(OrbitState);
// 		}
// 	}
// }

void APawnInSpace::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	DrawDebugDirectionalArrow
		( GetWorld()
		, MovableRoot->GetComponentLocation()
		, MovableRoot->GetComponentLocation() + 1000. * MovableRoot->GetForwardVector()
		, 20
		, FColor::Yellow
		);
	
	if(bIsAccelerating)
	{
		const FPhysics Physics = UStateLib::GetPhysicsUnsafe(this);
		const FPlayerUI PlayerUI = UStateLib::GetPlayerUIUnsafe
			(this
			, FLocalPlayerContext(GetGameInstance()->GetPrimaryPlayerController())
			);
		const float DeltaV = AccelerationSI / Physics.ScaleFactor * DeltaSeconds;
		Orbit->AddVelocity(MovableRoot->GetForwardVector() * DeltaV, Physics, PlayerUI);
	}
}

#if WITH_EDITOR
void APawnInSpace::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Orbit->SetCircleOrbit
		( UStateLib::GetPhysicsEditorDefault()
		, UStateLib::GetPlayerUIEditorDefault()
		);
}
#endif 

void APawnInSpace::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(APawnInSpace, RP_BodyRotation, COND_SkipOwner);
}