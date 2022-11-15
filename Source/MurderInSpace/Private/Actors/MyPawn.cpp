#include "Actors/MyPawn.h"

#include "Modes/MyGameState.h"
#include "Net/UnrealNetwork.h"

AMyPawn::AMyPawn()
{
	PrimaryActorTick.bCanEverTick = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);
    
	bNetLoadOnClient = false;
	bReplicates = true;
	AActor::SetReplicateMovement(false);
}

void AMyPawn::UpdateLookTarget(FVector Target)
{
	// TODO
}

void AMyPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	DrawDebugDirectionalArrow
		( GetWorld()
		, Root->GetComponentLocation()
		, Root->GetComponentLocation() + 1000. * Root->GetForwardVector()
		, 20
		, FColor::Yellow
		);
	
	if(RP_bIsAccelerating)
	{
		const FPhysics Physics = UStateLib::GetPhysicsUnsafe(this);
		const FInstanceUI InstanceUI = UStateLib::GetInstanceUIUnsafe(this);
		const float DeltaV = AccelerationSI / Physics.ScaleFactor * DeltaSeconds;
		Orbit->AddVelocity(Root->GetForwardVector() * DeltaV, Physics, InstanceUI);
	}
}

void AMyPawn::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ConstructOrbitForActor(this, true);
}

void AMyPawn::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
	
    const FPhysics Physics = UStateLib::GetPhysicsEditorDefault();
    const FInstanceUI InstanceUI = UStateLib::GetInstanceUIEditorDefault();
    
    const FName Name = PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName();

    static const FName FNameOrbitColor = GET_MEMBER_NAME_CHECKED(AMyPawn, OrbitColor);

	if(Name == FNameOrbitColor)
	{
		if(IsValid(Orbit))
		{
			Orbit->Update(Physics, InstanceUI);
		}
	}
}

void AMyPawn::BeginDestroy()
{
	Super::BeginDestroy();
	if(!IsValid(Orbit))
	{
		UE_LOG(LogMyGame, Warning, TEXT("%s: BeginDestroy: orbit invalid"), *GetFullName())
	}
	else
	{
		Orbit->Destroy();
	}
}

void AMyPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// two different ways to go about network lag and potential packet loss
	//
	// in case of rotation: "movement prediction": leaving the implementation to the client, allowing for quick movement;
	// in case this goes out of syn (COND_SkipOwner prevents re-sync), the player looks into the wrong direction for a while;
	//DOREPLIFETIME_CONDITION(APawnInSpace, RP_BodyRotation   , COND_SkipOwner)
	// just removing the COND_SkipOwner makes sure that the client's authority doesn't last more than a couple of frames
	DOREPLIFETIME(AMyPawn, RP_Rotation)

	// in case of acceleration: full server-control: the client won't react to the key press until the action has
	// round-tripped, i.e. there is no movement prediction
	DOREPLIFETIME(AMyPawn, RP_bIsAccelerating)
}
