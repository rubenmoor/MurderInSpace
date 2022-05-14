// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterInSpace.h"

#include <algorithm>

#include "AstronautHUD.h"

ACharacterInSpace::ACharacterInSpace()
{
	PrimaryActorTick.bCanEverTick = false;

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(Root);
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(Root);
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

	StarsClose = CreateDefaultSubobject<UNiagaraComponent>(TEXT("StarsClose"));
	StarsClose->SetupAttachment(StarAnchor);
	
	StarsDistant = CreateDefaultSubobject<UNiagaraComponent>(TEXT("StarsDistant"));
	StarsDistant->SetupAttachment(StarAnchor);
}

void ACharacterInSpace::UpdateSpringArm(uint8 CameraPosition)
{
	const auto Length = pow(CameraPosition, 2) * 250;
	SpringArm->TargetArmLength = Length;
	SpringArm->SetWorldRotation(FRotator(CameraPosition < 2 ? CameraPosition * -30 : -50 - CameraPosition * 5, 0, 0));
	StarAnchor->SetRelativeLocation(FVector(10000 + Length, 0, 0));
}

void ACharacterInSpace::SetVisibility(bool bVisibility)
{
	SkeletalMesh->SetVisibility(bVisibility, true);
}

void ACharacterInSpace::BeginPlay()
{
	Super::BeginPlay();
	
	if(const auto PlayerController = GetLocalViewingPlayerController())
	{
		HUD = PlayerController->GetHUD<AAstronautHUD>();
	}
	if(!HUD)
	{
		UE_LOG(LogPlayerController, Error, TEXT("ACharacterInSpace::BeginPlay: HUD null"))
		//RequestEngineExit("ACharachterInSpace::BeginPlay: HUD null");
	}
}
