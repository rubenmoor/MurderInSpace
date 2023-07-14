#include "Actors/MyPawn_Humanoid.h"

#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"

AMyPawn_Humanoid::AMyPawn_Humanoid()
{
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>("SkeletalMesh");
	SkeletalMesh->SetupAttachment(Root);
	SkeletalMesh->CanCharacterStepUpOn = ECB_No;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>("CapsuleComponent");
	CapsuleComponent->SetupAttachment(Root);
	CapsuleComponent->SetRelativeLocation({-6., 0., 19.});
	CapsuleComponent->SetCapsuleHalfHeight(70.);
	CapsuleComponent->SetCapsuleRadius(40.);
	
    CollisionComponent = CreateDefaultSubobject<UMyCollisionComponent>("Collision");
	
	// density is meant to represent the density of water at 1.
	// the sphere radius of the bounds of the astronaut is somewhat inflated, this is why the density is so low
	CollisionComponent->Density = 0.1;
	CollisionComponent->DensityExponent = 2.5;
}

void AMyPawn_Humanoid::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	CollisionComponent->UpdateMass(AMyPawn_Humanoid::GetBounds().SphereRadius);
}

void AMyPawn_Humanoid::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyPawn_Humanoid, RP_ActionState)
}
