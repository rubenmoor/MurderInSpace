// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Blackhole.h"

// Sets default values
ABlackhole::ABlackhole()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>("SceneRoot");
	SetRootComponent(Root);

	EventHorizon = CreateDefaultSubobject<UStaticMeshComponent>("EventHorizon");
	GravitationalLens = CreateDefaultSubobject<UStaticMeshComponent>("GravitationalLens");

	EventHorizon->SetupAttachment(Root);
	GravitationalLens->SetupAttachment(Root);
}
