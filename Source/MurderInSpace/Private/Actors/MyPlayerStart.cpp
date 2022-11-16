// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/MyPlayerStart.h"

#include "Actors/Orbit.h"

void AMyPlayerStart::InitializeOrbit()
{
	SpawnOrbit(this);
}

void AMyPlayerStart::Destroyed()
{
	Super::Destroyed();
	
	if(!IsValid(Orbit))
	{
		UE_LOG(LogMyGame, Warning, TEXT("%s: BeginDestroy: orbit invalid"), *GetFullName())
	}
	else
	{
		Orbit->Destroy();
	}
}

void AMyPlayerStart::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	OrbitOnConstruction(this, false);
}
