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
}

void AActorInSpace::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Orbit->InitializeCircle
		( MovableRoot->GetComponentLocation()
		, UStateLib::GetPhysicsEditorDefault()
		, UStateLib::GetPlayerUIEditorDefault()
		);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AActorInSpace::HandleBeginMouseOver(UPrimitiveComponent*)
{
	Orbit->bIsVisibleVarious = true;
	Orbit->UpdateVisibility(UStateLib::GetPlayerUIUnsafe(this));
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AActorInSpace::HandleEndMouseOver(UPrimitiveComponent*)
{
	Orbit->bIsVisibleVarious = false;
	Orbit->UpdateVisibility(UStateLib::GetPlayerUIUnsafe(this));
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AActorInSpace::HandleClick(UPrimitiveComponent*, FKey Button)
{
	if(Button == EKeys::LeftMouseButton)
	{
		Orbit->bIsSelected = !Orbit->bIsSelected;
	}
}