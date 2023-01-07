// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/MyCharacter.h"

#include "NiagaraComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Lib/FunctionLib.h"

AMyCharacter::AMyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

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

	Visor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visor"));
	Visor->SetupAttachment(Camera);
	Visor->SetRelativeLocation(FVector(10, 0, 0));
	Visor->SetRelativeScale3D(FVector(0.8, 1.3, 1.3));
	Visor->SetGenerateOverlapEvents(false);
	Visor->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Visor->SetVisibility(false);

	VisorFrame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisorFrame"));
	VisorFrame->SetupAttachment(Visor);
	VisorFrame->SetRelativeLocation(FVector(25.0, 0, 0));
	VisorFrame->SetRelativeRotation(FRotator(90, 0, 0));
	VisorFrame->SetGenerateOverlapEvents(false);
	VisorFrame->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisorFrame->SetVisibility(false);

	StarAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("StarAnchor"));
	StarAnchor->SetupAttachment(SpringArm);
	StarAnchor->SetRelativeLocation(FVector(11000, 0, 0));

	StarsClose = CreateDefaultSubobject<UNiagaraComponent>(TEXT("StarsClose"));
	StarsClose->SetupAttachment(StarAnchor);
	StarsClose->SetVisibility(false);
	
	StarsDistant = CreateDefaultSubobject<UNiagaraComponent>(TEXT("StarsDistant"));
	StarsDistant->SetupAttachment(StarAnchor);
	StarsDistant->SetRelativeLocation(FVector(10000, 0, 0));
	StarsDistant->SetVisibility(false);
}

void AMyCharacter::UpdateSpringArm(uint8 CameraPosition)
{
	const float Length = pow(CameraPosition, 2) * 250;
	SpringArm->TargetArmLength = Length;
	SpringArm->SetWorldRotation(FRotator
		( CameraPosition < 2 ? -10 + CameraPosition * -20 : -50 - CameraPosition * 5
		, 0
		, 0
		));
	StarAnchor->SetRelativeLocation(FVector(10000 + Length, 0, 0));
	// TODO: maybe missing an orbit component?
	for(MyObjectIterator<AOrbit> IOrbit(GetWorld()); IOrbit; ++IOrbit)
	{
		(*IOrbit)->UpdateSplineMeshScale(Length / 1000.);
	}
}

void AMyCharacter::ShowEffects()
{
	Visor->SetVisibility(true);
	VisorFrame->SetVisibility(true);
	StarsClose->SetVisibility(true);
	StarsDistant->SetVisibility(true);
}

void AMyCharacter::SetVisibility(bool bVisibility)
{
	SkeletalMesh->SetVisibility(bVisibility, true);
}

float AMyCharacter::GetSpringArmLength() const
{
	return SpringArm->TargetArmLength;
}

void AMyCharacter::OnConstruction(const FTransform& Transform)
{
	APawn::OnConstruction(Transform);
    if  (
		// Only the server spawns orbits
    	   GetLocalRole()        == ROLE_Authority
    	   
		// avoid orbit spawning when editing and compiling blueprint
		&& GetWorld()->WorldType != EWorldType::EditorPreview
		)
    {
		OrbitSetup(this);
    }
}
