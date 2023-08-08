#include "Spacebodies/MyPawn_Humanoid.h"

#include "Components/CapsuleComponent.h"
#include "GameplayAbilitySystem/MyDeveloperSettings.h"
#include "Logging/StructuredLog.h"
#include "Modes/MyGameInstance.h"
#include "SpaceItems/HandThruster.h"

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

void AMyPawn_Humanoid::BeginPlay()
{
	Super::BeginPlay();

	auto* Settings = GetDefault<UMyDeveloperSettings>();
	
	HandThrusterLeft = GetWorld()->SpawnActor<AHandThruster>(Settings->HandThrusterInitialClass);
	HandThrusterLeft->AttachToComponent(SkeletalMesh, FAttachmentTransformRules::KeepRelativeTransform, "SocketLeftHand");
	HandThrusterLeft->SetActorHiddenInGame(true);
	
	HandThrusterRight = GetWorld()->SpawnActor<AHandThruster>(Settings->HandThrusterInitialClass);
	HandThrusterRight->AttachToComponent(SkeletalMesh, FAttachmentTransformRules::KeepRelativeTransform, "SocketRightHand");
	HandThrusterRight->SetActorHiddenInGame(true);
}

void AMyPawn_Humanoid::GameplayCue_Accelerate_ShowThrusters(FGameplayTag Cue, EGameplayCueEvent::Type Event,
	const FGameplayCueParameters& Parameters)
{
	switch (Event)
	{
	case EGameplayCueEvent::OnActive:
	case EGameplayCueEvent::WhileActive:
		HandThrusterLeft->SetActorHiddenInGame(false);
		HandThrusterRight->SetActorHiddenInGame(false);
		break;
	case EGameplayCueEvent::Executed:
		UE_LOGFMT(LogMyGame, Error, "{CUE}: Executed: not implemented", Cue.GetTagName());
		break;
	case EGameplayCueEvent::Removed:
		HandThrusterLeft->SetActorHiddenInGame(true);
		HandThrusterRight->SetActorHiddenInGame(true);
		break;
	}
}

void AMyPawn_Humanoid::GameplayCue_Accelerate_Fire(FGameplayTag Cue, EGameplayCueEvent::Type Event,
	const FGameplayCueParameters& Parameters)
{
	switch (Event)
	{
	case EGameplayCueEvent::OnActive:
	case EGameplayCueEvent::WhileActive:
		HandThrusterLeft->EnableBurn(true);
		HandThrusterRight->EnableBurn(true);
		break;
	case EGameplayCueEvent::Executed:
		UE_LOGFMT(LogMyGame, Error, "{CUE}: Executed: not implemented", Cue.GetTagName());
		break;
	case EGameplayCueEvent::Removed:
		HandThrusterLeft->EnableBurn(false);
		HandThrusterRight->EnableBurn(false);
		break;
	}
}