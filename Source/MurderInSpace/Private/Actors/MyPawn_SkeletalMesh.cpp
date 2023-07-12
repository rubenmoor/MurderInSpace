#include "Actors/MyPawn_SkeletalMesh.h"

#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"

AMyPawn_SkeletalMesh::AMyPawn_SkeletalMesh()
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
	CollisionComponent->UpdateMass(AMyPawn_SkeletalMesh::GetBounds().SphereRadius);
}

void AMyPawn_SkeletalMesh::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyPawn_SkeletalMesh, RP_bActionEmbrace)
	DOREPLIFETIME(AMyPawn_SkeletalMesh, RP_bActionIdle)
	DOREPLIFETIME(AMyPawn_SkeletalMesh, RP_bActionRotate)
	DOREPLIFETIME(AMyPawn_SkeletalMesh, RP_RotationDirection)
	DOREPLIFETIME(AMyPawn_SkeletalMesh, RP_bActionKickExecute)
	DOREPLIFETIME(AMyPawn_SkeletalMesh, RP_bActionKickPosition)
}