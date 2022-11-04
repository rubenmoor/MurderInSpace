#include "Actors/ActorInSpace.h"

#include "Actors/GyrationComponent.h"
#include "Actors/OrbitComponent.h"

AActorInSpace::AActorInSpace()
{
	PrimaryActorTick.bCanEverTick = true;

	// components

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Root->SetMobility(EComponentMobility::Stationary);
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
	
	Gyration = CreateDefaultSubobject<UGyrationComponent>(TEXT("Gyration"));

	bReplicates = true;
	AActor::SetReplicateMovement(false);
}

void AActorInSpace::OnConstruction(const FTransform& Transform)
{
	// TODO: it looks like, contrary to documentation, `OnConstruction` is called twice:
	// once when an ActorInSpace is placed in the editor (which is fine)
	// a second time on game start, which is wrong
	Super::OnConstruction(Transform);
	Orbit->SetCircleOrbit
		( MovableRoot->GetComponentLocation()
		, UStateLib::GetPhysicsEditorDefault()
		, UStateLib::GetPlayerUIEditorDefault()
		);
}

void AActorInSpace::BeginPlay()
{
	Super::BeginPlay();
	if(GetLocalRole() == ROLE_Authority)
	{
		Orbit->SetCircleOrbit
			( MovableRoot->GetComponentLocation()
			, UStateLib::GetPhysicsUnsafe(this)
			, UStateLib::GetPlayerUIEditorDefault()
			);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AActorInSpace::HandleBeginMouseOver(UPrimitiveComponent*)
{
	// cheating here: for split screen, we would need to track orbit visibility in the player state
	Orbit->bIsVisibleVarious = true;
	// ... and cheating again: assuming mouse belongs to primary player controller
	Orbit->UpdateVisibility(UStateLib::GetPlayerUIUnsafe
		(this
		, FLocalPlayerContext(GetGameInstance()->GetPrimaryPlayerController())
		));
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AActorInSpace::HandleEndMouseOver(UPrimitiveComponent*)
{
	Orbit->bIsVisibleVarious = false;
	Orbit->UpdateVisibility(UStateLib::GetPlayerUIUnsafe
		(this
		, FLocalPlayerContext(GetGameInstance()->GetPrimaryPlayerController())
		));
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AActorInSpace::HandleClick(UPrimitiveComponent*, FKey Button)
{
	if(Button == EKeys::LeftMouseButton)
	{
		Orbit->bIsSelected = !Orbit->bIsSelected;
	}
}
