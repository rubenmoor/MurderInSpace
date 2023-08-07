#include "Spacebodies/MyPlayerStart.h"

#include "Components/CapsuleComponent.h"
#include "Orbit/Orbit.h"

AMyPlayerStart::AMyPlayerStart()
{
	bNetLoadOnClient = false;
	bReplicates = false;
	AActor::SetReplicateMovement(false);

	CollisionCapsule = CreateDefaultSubobject<UCapsuleComponent>("CollisionCapsule");
	SetRootComponent(CollisionCapsule);
	CollisionCapsule->SetCapsuleRadius(100.);
	CollisionCapsule->SetCapsuleHalfHeight(100.);
}

void AMyPlayerStart::Destroyed()
{
	Super::Destroyed();
	if(IsValid(Orbit))
	{
		Orbit->Destroy();
	}
}

void AMyPlayerStart::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
    if  (
		// avoid orbit spawning when editing and compiling blueprint
		   GetWorld()->WorldType != EWorldType::EditorPreview
		
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
