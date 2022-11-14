#include "Actors/MyPawn_SkeletalMesh.h"

AMyPawn_SkeletalMesh::AMyPawn_SkeletalMesh()
{
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(Root);
}
