// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/WorldBoundaries.h"

#include "Modes/MyState.h"

// Sets default values
AWorldBoundaries::AWorldBoundaries()
{
	PrimaryActorTick.bCanEverTick = false;
	AActor::SetActorHiddenInGame(true);
	
    Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
    Sphere->SetMobility(EComponentMobility::Stationary);
	SetRootComponent(Sphere);
}

void AWorldBoundaries::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Sphere->SetSphereRadius(PhysicsEditorDefault.WorldRadius);
}

// Called when the game starts or when spawned
void AWorldBoundaries::BeginPlay()
{
	Super::BeginPlay();
	UMyState* MyState = GEngine->GetEngineSubsystem<UMyState>();
	MyState->WithPhysics(this, [this] (FPhysics& Physics)
	{
		Sphere->SetSphereRadius(Physics.WorldRadius);
	});
}

// Called every frame
void AWorldBoundaries::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

