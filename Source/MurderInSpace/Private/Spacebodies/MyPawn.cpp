#include "Spacebodies/MyPawn.h"

#include "Spacebodies/MyCharacter.h"
#include "GameplayAbilitySystem/MyAttributes.h"
#include "GameplayAbilitySystem/MyAbilitySystemComponent.h"
#include "MyGameplayTags.h"
#include "GameplayAbilitySystem/MyDeveloperSettings.h"
#include "HUD/MyHUD.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Logging/StructuredLog.h"
#include "Modes/MyGameInstance.h"
#include "Modes/MyGameState.h"
#include "Modes/MyPlayerController.h"
#include "Net/UnrealNetwork.h"

AMyPawn::AMyPawn(): APawn()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	bNetLoadOnClient = false;
	bReplicates = true;
	// TODO: not sure if necessary, but not harmful either
	bAlwaysRelevant = true;
	AActor::SetReplicateMovement(false);

	Root = CreateDefaultSubobject<USceneComponent>("Root");
	SetRootComponent(Root);

	AbilitySystemComponent = CreateDefaultSubobject<UMyAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->ReplicationMode = EGameplayEffectReplicationMode::Mixed;
}

void AMyPawn::SetOmega(float InOmega)
{
	const float Delta = InOmega - Omega;
	if(FMath::Abs(Delta) > 0.05)
		UE_LOGFMT(LogMyGame, Error, "Setting Omega to {NEW}: old Omega = {OMEGA}; Delta = {DELTA}, |Delta| > 0.05"
			, InOmega, Omega, Delta);
	Omega = InOmega;
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
	
	const UWorld* World = GetWorld();
	const auto* GS = World->GetGameState<AMyGameState>();
	const FPhysics Physics = GS->RP_Physics;

	const double A = AttrSetAcceleration->GetAccelerationSI();
	if(A != 0.)
	{
		const double DeltaV = A / FPhysics::LengthScaleFactor * DeltaSeconds;
		RP_Orbit->Update(GetActorForwardVector() * DeltaV, Physics);
	}
	
	Omega += AttrSetAcceleration->GetTorque() * DeltaSeconds;
	SetActorRotation(GetActorQuat() * FQuat(FVector::UnitZ(), Omega * DeltaSeconds));
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
		&& (!HasAnyFlags(RF_Transient) || Cast<AMyCharacter>(this))
		)
	{
		OrbitSetup(this);
	}
}

void AMyPawn::BeginPlay()
{
	Super::BeginPlay();

    if(GetLocalRole() == ROLE_Authority)
    {
		Initialize();
    }
    else
    {
        SetReadyFlags(EMyPawnReady::InternalReady);
    }
}

void AMyPawn::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	AttrSetAcceleration = NewObject<UAttrSetAcceleration>(this, "AttrSetAcceleration");
}

void AMyPawn::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	for(const auto Ability : StartupAbilities)
	{
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability));
	}
	
}

void AMyPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	SetOwner(NewController);
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
			RP_Orbit->Update(FPhysics());
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
		SetReadyFlags(EMyPawnReady::OrbitReady);
	}
}

void AMyPawn::SetReadyFlags(EMyPawnReady ReadyFlags)
{
	MyPawnReady |= ReadyFlags;
	if(MyPawnReady == EMyPawnReady::All)
	{
		Initialize();
	}
}

void AMyPawn::Initialize()
{
	SetActorTickEnabled(true);
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
}
