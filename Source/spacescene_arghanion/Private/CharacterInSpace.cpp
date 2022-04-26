// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterInSpace.h"

ACharacterInSpace::ACharacterInSpace()
{
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(MeshRoot);
	SpringArm->TargetArmLength = 1000;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 3;
	SpringArm->bDoCollisionTest = false;
	SpringArm->SetUsingAbsoluteRotation(true);
	SpringArm->SetWorldRotation(FRotator(300,0,0));

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
}

void ACharacterInSpace::UpdateSpringArm(uint8 CameraPosition)
{
	SpringArm->TargetArmLength = pow(CameraPosition, 2) * 250;
	SpringArm->SetWorldRotation(FRotator(CameraPosition < 2 ? CameraPosition * -30 : -50 - CameraPosition * 5, 0, 0));
}

void ACharacterInSpace::SetVisibility(bool bVisibility)
{
	MeshRoot->SetVisibility(bVisibility, true);
}
