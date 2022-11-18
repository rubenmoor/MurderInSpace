#include "Actors/MyPawn.h"

#include "Modes/MyGameState.h"
#include "Net/UnrealNetwork.h"

AMyPawn::AMyPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	bNetLoadOnClient = false;
	bReplicates = true;
	// TODO: not sure if necessary, but not harmful either
    bAlwaysRelevant = true;
	AActor::SetReplicateMovement(false);

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);
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
		UMyState* MyState = GEngine->GetEngineSubsystem<UMyState>();
		const FPhysics Physics = MyState->GetPhysicsAny(this);
		const FInstanceUI InstanceUI = MyState->GetInstanceUIAny(this);
		const float DeltaV = AccelerationSI / Physics.ScaleFactor * DeltaSeconds;
		Cast<AOrbit>(Children[0])->AddVelocity(Root->GetForwardVector() * DeltaV, Physics, InstanceUI);
	}
}

void AMyPawn::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	OrbitSetup(this);
}

#if WITH_EDITOR
void AMyPawn::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	UMyState* MyState = GEngine->GetEngineSubsystem<UMyState>();
    const FPhysics Physics = MyState->GetPhysicsEditorDefault();
    const FInstanceUI InstanceUI = MyState->GetInstanceUIEditorDefault();
    
    const FName Name = PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName();

    static const FName FNameOrbitColor = GET_MEMBER_NAME_CHECKED(AMyPawn, OrbitColor);

	if(Name == FNameOrbitColor)
	{
		if(!Children.IsEmpty())
		{
			Cast<AOrbit>(Children[0])->Update(Physics, InstanceUI);
		}
	}
}
#endif

void AMyPawn::Destroyed()
{
	Super::Destroyed();
	while(Children.Num() > 0)
	{
		Children.Last()->Destroy();
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
