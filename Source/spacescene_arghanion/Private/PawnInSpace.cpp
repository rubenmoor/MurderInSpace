// Fill out your copyright notice in the Description page of Project Settings.


#include "PawnInSpace.h"

#include "MyGameInstance.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
APawnInSpace::APawnInSpace()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// components

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(Root);

	MeshRoot = CreateDefaultSubobject<USceneComponent>(TEXT("MeshRoot"));
	MeshRoot->SetupAttachment(Root);
	
	//Orbit = new UKeplerOrbitComponent(FVector::Zero(), MeshRoot);
	Orbit = CreateDefaultSubobject<UKeplerOrbitComponent>(TEXT("KeplerOrbit"));
	Orbit->Initialize(FVector::Zero(), MeshRoot);
	Orbit->SetupAttachment(Root);
}

void APawnInSpace::UpdateMU(float MU) const
{
	Orbit->UpdateOrbit(MU);
}

void APawnInSpace::UpdateLookTarget(FVector Target)
{
	// TODO
}

void APawnInSpace::LookAt(FVector VecP)
{
	const auto VecMe = MeshRoot->GetComponentLocation();
	const auto VecDirection = VecP - VecMe;
	const auto Quat = FQuat::FindBetween(FVector(0, 1, 0), VecDirection);
	const auto AngleDelta = Quat.GetAngle() - MeshRoot->GetComponentQuat().GetAngle();
	DrawDebugDirectionalArrow(GetWorld(), VecMe, VecP, 20, FColor::Red);
	if(abs(AngleDelta) > 15. / 180 * PI)
	{
		MeshRoot->SetWorldRotation(Quat);
	}
}

void APawnInSpace::BeginPlay()
{
	Orbit->UpdateOrbit(GetGameInstance<UMyGameInstance>()->MU);
	Super::BeginPlay();
}

void APawnInSpace::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Orbit->UpdateOrbit(UKeplerOrbitComponent::DefaultMU);
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
