// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/MyCharacter.h"

#include "NiagaraComponent.h"
#include "Actors/MyPlayerStart.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/SpringArmComponent.h"
#include "HUD/MyHUD.h"
#include "Lib/FunctionLib.h"
#include "Modes/MyPlayerController.h"

AMyCharacter::AMyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	Dust = CreateDefaultSubobject<UNiagaraComponent>("Dust");
	Dust->SetupAttachment(Root);
	Dust->SetVisibility(false);
	Dust->SetVariableLinearColor("Color", FLinearColor(FVector4d(1., 1., 1., .5)));
	Dust->SetVariableFloat("SizeMax", 10.);
	Dust->SetVariableFloat("SizeMin", 5.);
	Dust->SetVariableFloat("SpawnCylinderRadius", 2000.);
	Dust->SetVariableFloat("SpawnProbability", .1);
	Dust->SetVariableFloat("SpawnRate", 20.);
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(Root);
	SpringArm->TargetArmLength = 1000;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 3;
	SpringArm->bDoCollisionTest = false;
	SpringArm->SetUsingAbsoluteRotation(true);
	SpringArm->SetWorldRotation(FRotator(300,0,0));

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm);

	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>("SceneCapture");
	SceneCapture->SetupAttachment(Camera);

	Visor = CreateDefaultSubobject<UStaticMeshComponent>("Visor");
	Visor->SetupAttachment(Camera);
	Visor->SetRelativeLocation(FVector(10, 0, 0));
	Visor->SetRelativeScale3D(FVector(0.8, 1.3, 1.3));
	Visor->SetGenerateOverlapEvents(false);
	Visor->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Visor->SetVisibility(false);

	VisorFrame = CreateDefaultSubobject<UStaticMeshComponent>("VisorFrame");
	VisorFrame->SetupAttachment(Visor);
	VisorFrame->SetRelativeLocation(FVector(25.0, 0, 0));
	VisorFrame->SetRelativeRotation(FRotator(90, 0, 0));
	VisorFrame->SetGenerateOverlapEvents(false);
	VisorFrame->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisorFrame->SetVisibility(false);

	StarAnchor = CreateDefaultSubobject<USceneComponent>("StarAnchor");
	StarAnchor->SetupAttachment(SpringArm);
	StarAnchor->SetRelativeLocation(FVector(20000, 0, 0));

	StarsClose = CreateDefaultSubobject<UNiagaraComponent>("StarsClose");
	StarsClose->SetupAttachment(StarAnchor);
	StarsClose->SetVisibility(false);
	StarsClose->SetVariableLinearColor("Color", FLinearColor(FVector4d(1., 1., 1., 1.)));
	StarsClose->SetVariableFloat("SizeMax", 60.);
	StarsClose->SetVariableFloat("SizeMin", 10.);
	StarsClose->SetVariableFloat("SpawnCylinderRadius", 10000.);
	StarsClose->SetVariableFloat("SpawnProbability", .1);
	StarsClose->SetVariableFloat("SpawnRate", 25.);
	
	StarsDistant = CreateDefaultSubobject<UNiagaraComponent>("StarsDistant");
	StarsDistant->SetupAttachment(StarAnchor);
	StarsDistant->SetRelativeLocation(FVector(40000, 0, 0));
	StarsDistant->SetVisibility(false);
	StarsDistant->SetVariableLinearColor("Color", FLinearColor(FVector4d(1., 1., 1., 1.)));
	StarsDistant->SetVariableFloat("SizeMax", 120.);
	StarsDistant->SetVariableFloat("SizeMin", 30.);
	StarsDistant->SetVariableFloat("SpawnCylinderRadius", 50000.);
	StarsDistant->SetVariableFloat("SpawnProbability", .1);
	StarsDistant->SetVariableFloat("SpawnRate", 50.);

	// make collisions with character inelastic
	CollisionComponent->CoR = 0.2;
}

void AMyCharacter::UpdateSpringArm(uint8 InCameraPosition)
{
	CameraPosition = InCameraPosition;
	const float Length = CameraLengthFactor * (1. + pow(CameraPosition, CameraLengthExponent)) + CameraLengthConst;
	SpringArm->TargetArmLength = Length;
	SpringArm->SetWorldRotation(FRotator
		( -CameraRotationConst + CameraPosition * -CameraRotationFactor
		, 0
		, 0
		));
	//StarAnchor->SetRelativeLocation(FVector(10000 + Length, 0, 0));
	// TODO: redundant when orbits are nativepaint
	for(TMyObjectIterator<AOrbit> IOrbit(GetWorld()); IOrbit; ++IOrbit)
	{
		(*IOrbit)->UpdateSplineMeshScale(sqrt(Length) / 100.);
	}
}

void AMyCharacter::ShowEffects()
{
	Visor->SetVisibility(true);
	VisorFrame->SetVisibility(true);
	StarsClose->SetVisibility(true);
	StarsDistant->SetVisibility(true);
	Dust->SetVisibility(true);
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

void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();
	auto Res = GEngine->GetGameUserSettings()->GetScreenResolution();
    auto* RenderTarget = SetNewRenderTarget(Res.X, Res.Y);
	GetController<AMyPlayerController>()->GetHUD<AMyHUD>()->SetOrbitMaterial(RenderTarget);
}

#if WITH_EDITOR
void AMyCharacter::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	const FName Name = PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName();

	static const FName FNameCameraLengthExponent = GET_MEMBER_NAME_CHECKED(AMyCharacter, CameraLengthExponent);
	static const FName FNameCameraLengthConst    = GET_MEMBER_NAME_CHECKED(AMyCharacter, CameraLengthConst   );
	static const FName FNameCameraLengthFactor   = GET_MEMBER_NAME_CHECKED(AMyCharacter, CameraLengthFactor  );
	static const FName FNameCameraRotationConst  = GET_MEMBER_NAME_CHECKED(AMyCharacter, CameraRotationConst );
	static const FName FNameCameraRotationFactor = GET_MEMBER_NAME_CHECKED(AMyCharacter, CameraRotationFactor);
	static const FName FNameCameraPosition       = GET_MEMBER_NAME_CHECKED(AMyCharacter, CameraPosition      );

	if  (  Name == FNameCameraLengthExponent
		|| Name == FNameCameraLengthConst
		|| Name == FNameCameraLengthFactor
		|| Name == FNameCameraRotationConst
		|| Name == FNameCameraRotationFactor
		|| Name == FNameCameraPosition
		)
	{
		UpdateSpringArm(CameraPosition);
	}
}
#endif
