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

	bNetLoadOnClient = false;
	bReplicates = true;
	AActor::SetReplicateMovement(false);
}

void AActorInSpace::OnConstruction(const FTransform& Transform)
{
	if(Orbit->bSkipConstruction)
	{
		return;
	}
	
	// TODO: it looks like, contrary to documentation, `OnConstruction` is called twice:
	// once when an ActorInSpace is placed in the editor (which is fine)
	// a second time on game start, which is wrong
	Super::OnConstruction(Transform);

	const FPhysics Physics = UStateLib::GetPhysicsEditorDefault();
	const FInstanceUI InstanceUI = UStateLib::GetInstanceUIEditorDefault();
	
	if(!Orbit->GetHasBeenSet())
	{
		Orbit->SetCircleOrbit(Transform.GetLocation(), Physics);
	}
	Orbit->Update(Physics, InstanceUI);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AActorInSpace::HandleBeginMouseOver(UPrimitiveComponent*)
{
	// cheating here: for split screen, we would need to track orbit visibility in the player state
	Orbit->bIsVisibleVarious = true;
	// ... and cheating again: assuming mouse belongs to primary player controller
	Orbit->UpdateVisibility(UStateLib::GetInstanceUIUnsafe(this));
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AActorInSpace::HandleEndMouseOver(UPrimitiveComponent*)
{
	Orbit->bIsVisibleVarious = false;
	Orbit->UpdateVisibility(UStateLib::GetInstanceUIUnsafe(this));
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AActorInSpace::HandleClick(UPrimitiveComponent*, FKey Button)
{
	if(Button == EKeys::LeftMouseButton)
	{
		Orbit->bIsSelected = !Orbit->bIsSelected;
	}
}
