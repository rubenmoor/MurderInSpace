#include "Actors/MyPawn.h"

#include "HUD/MyHUD.h"
#include "Modes/MyGameInstance.h"
#include "Modes/MyGameState.h"
#include "Modes/MyPlayerController.h"
#include "Net/UnrealNetwork.h"

AMyPawn::AMyPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	bNetLoadOnClient = false;
	bReplicates = true;
	// TODO: not sure if necessary, but not harmful either
    bAlwaysRelevant = true;
	AActor::SetReplicateMovement(false);

    Root = CreateDefaultSubobject<USceneComponent>("Root");
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
		, GetActorLocation()
		, GetActorLocation() + 1000. * GetActorForwardVector()
		, 20
		, FColor::Yellow
		);

	UMyState* MyState = GEngine->GetEngineSubsystem<UMyState>();
	const UWorld* World = GetWorld();
	const auto* GS = World->GetGameState<AMyGameState>();
	const FPhysics Physics = MyState->GetPhysics(GS);
	const auto* GI = GetGameInstance<UMyGameInstance>();
	const FInstanceUI InstanceUI = MyState->GetInstanceUI(GI);
	
	if(RP_bIsAccelerating && IsValid(RP_Orbit))
	{
		const double DeltaV = AccelerationSI / Physics.ScaleFactor * DeltaSeconds;
		RP_Orbit->Update(GetActorForwardVector() * DeltaV, Physics, InstanceUI, true);
		bWasAccelerating = true;
	}
	else
	{
		if(bWasAccelerating)
		{
			RP_Orbit->Update(Physics, InstanceUI);
		}
		bWasAccelerating = false;
	}
}

void AMyPawn::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
    if  (
		// Only the server spawns orbits
    	   GetLocalRole()        == ROLE_Authority
    	   
		// avoid orbit spawning when editing and compiling blueprint
		&& GetWorld()->WorldType != EWorldType::EditorPreview
		
		// avoid orbit spawning when dragging an actor with orbit into the viewport at first
		// The preview actor that is created doesn't have a valid location
		// Once the actor is placed inside the viewport, it's no longer transient and the orbit is reconstructed properly
		// according to the actor location
		&& !HasAnyFlags(RF_Transient)
		)
    {
		OrbitSetup(this);
    }
}

#if WITH_EDITOR
void AMyPawn::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

    const FName Name = PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName();

    static const FName FNameOrbitColor = GET_MEMBER_NAME_CHECKED(AMyPawn, OrbitColor);

	if(Name == FNameOrbitColor)
	{
		if(IsValid(RP_Orbit))
		{
			RP_Orbit->Update(PhysicsEditorDefault, InstanceUIEditorDefault);
		}
	}
}
#endif

void AMyPawn::Destroyed()
{
	Super::Destroyed();
	if(IsValid(RP_Orbit))
	{
		RP_Orbit->Destroy();
	}
}

void AMyPawn::OnRep_Orbit()
{
	if(GetLocalRole() == ROLE_AutonomousProxy)
	{
		GetController<AMyPlayerController>()->GetHUD<AMyHUD>()->SetReadyFlags(EHUDReady::PawnOrbitReady);
	}
}

void AMyPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(AMyPawn, RP_Orbit, COND_InitialOnly)
	
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
