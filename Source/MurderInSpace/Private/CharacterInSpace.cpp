// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterInSpace.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "NiagaraComponent.h"

ACharacterInSpace::ACharacterInSpace()
{
	PrimaryActorTick.bCanEverTick = true;

	TempSplineMeshParent = CreateDefaultSubobject<USceneComponent>(TEXT("TempSplineMesh"));
	TempSplineMeshParent->SetupAttachment(Orbit);

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(MovableRoot);
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(MovableRoot);
	SpringArm->TargetArmLength = 1000;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 3;
	SpringArm->bDoCollisionTest = false;
	SpringArm->SetUsingAbsoluteRotation(true);
	SpringArm->SetWorldRotation(FRotator(300,0,0));

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	StarAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("StarAnchor"));
	StarAnchor->SetupAttachment(SpringArm);
	StarAnchor->SetRelativeLocation(FVector(11000, 0, 0));

	StarsClose = CreateDefaultSubobject<UNiagaraComponent>(TEXT("StarsClose"));
	StarsClose->SetupAttachment(StarAnchor);
	
	StarsDistant = CreateDefaultSubobject<UNiagaraComponent>(TEXT("StarsDistant"));
	StarsDistant->SetupAttachment(StarAnchor);
	StarsDistant->SetRelativeLocation(FVector(10000, 0, 0));

	// for the editor
	Orbit->UpdateVisibility(UStateLib::GetPlayerUIEditorDefault());
}

void ACharacterInSpace::UpdateSpringArm(uint8 CameraPosition)
{
	const float Length = pow(CameraPosition, 2) * 250;
	SpringArm->TargetArmLength = Length;
	SpringArm->SetWorldRotation(FRotator
		( CameraPosition < 2 ? -10 + CameraPosition * -20 : -50 - CameraPosition * 5
		, 0
		, 0
		));
	StarAnchor->SetRelativeLocation(FVector(10000 + Length, 0, 0));
	for(TObjectIterator<UOrbitComponent> Iter; Iter; ++Iter)
	{
		(*Iter)->UpdateSplineMeshScale(Length / 1000.);
	}
}

void ACharacterInSpace::SetVisibility(bool bVisibility)
{
	SkeletalMesh->SetVisibility(bVisibility, true);
}

float ACharacterInSpace::GetSpringArmLength() const
{
	return SpringArm->TargetArmLength;
}

void ACharacterInSpace::DestroyTempSplineMesh()
{
	TArray<USceneComponent*> Meshes;
	TempSplineMeshParent->GetChildrenComponents(false, Meshes);
	for(USceneComponent* const Mesh : Meshes)
	{
		Mesh->DestroyComponent();
	}
}
