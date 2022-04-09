// Fill out your copyright notice in the Description page of Project Settings.


#include "PawnInSpace.h"

#include "MyGameInstance.h"

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

// Called when the game starts or when spawned
void APawnInSpace::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APawnInSpace::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APawnInSpace::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void APawnInSpace::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Orbit->UpdateOrbit(UKeplerOrbitComponent::DefaultMU);
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
