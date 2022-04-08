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
	
	MainMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainMesh"));
	MainMesh->SetupAttachment(MeshRoot);

	//Orbit = new UKeplerOrbitComponent(FVector::Zero(), MeshRoot);
	Orbit = CreateDefaultSubobject<UKeplerOrbitComponent>(TEXT("KeplerOrbit"));
	Orbit->Initialize(FVector::Zero(), MeshRoot);
	Orbit->SetupAttachment(Root);
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

void APawnInSpace::OnConstruction(const FTransform& Transform)
{
	const auto MU = UKeplerOrbitComponent::DefaultMU;
	const auto VecR = Transform.GetLocation();
	const auto VecV = 1.0 * sqrt(MU / VecR.Length()) * (FVector(.2, 0, 0) + FVector(0.1, 0.2, 0.9).Cross(VecR.GetSafeNormal()));
	Orbit->UpdateOrbit(VecR, VecV, MU);
	Super::OnConstruction(Transform);
}

