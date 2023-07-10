#include "Actors/MyPawn_SkeletalMesh.h"

#include "Components/CapsuleComponent.h"

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
}
