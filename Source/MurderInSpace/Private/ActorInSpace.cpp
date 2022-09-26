#include "ActorInSpace.h"

#include "MyGameInstance.h"

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

	Gyration = CreateDefaultSubobject<UGyrationComponent>(TEXT("Gyration"));
}

void AActorInSpace::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Orbit->InitializeCircle
		( UMyGameInstance::EditorDefaultAlpha
		, UMyGameInstance::EditorDefaultWorldRadiusUU
		, UMyGameInstance::EditorDefaultVecF1
		, MovableRoot->GetComponentLocation()
		);
}

void AActorInSpace::HandleBeginMouseOver(UPrimitiveComponent*)
{
	GEngine->AddOnScreenDebugMessage(-1, 3., FColor::White, TEXT("begin mouse hover"));
	Orbit->SetVisibility(true, true);
	Orbit->bIsVisible = true;
}

void AActorInSpace::HandleEndMouseOver(UPrimitiveComponent*)
{
	GEngine->AddOnScreenDebugMessage(-1, 3., FColor::White, TEXT("end mouse hover"));
	if(!Orbit->bIsSelected)
	{
		Orbit->SetVisibility(false, true);
		Orbit->bIsVisible = false;
	}
}

void AActorInSpace::HandleClick(UPrimitiveComponent*, FKey Button)
{
	GEngine->AddOnScreenDebugMessage(-1, 3., FColor::White, TEXT("clicked"));
	if(Button == EKeys::LeftMouseButton)
	{
		Orbit->bIsSelected = !Orbit->bIsSelected;
	}
}
