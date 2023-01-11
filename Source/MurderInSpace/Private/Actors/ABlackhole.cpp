// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/ABlackhole.h"

// Sets default values
AABlackhole::AABlackhole()
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
