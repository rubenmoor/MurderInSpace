// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/MyPlayerStart.h"

#include "Actors/Orbit.h"

void AMyPlayerStart::Destroyed()
{
	Super::Destroyed();
	while(Children.Num() > 0)
	{
		Children.Last()->Destroy();
	}
}

void AMyPlayerStart::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	OrbitOnConstruction(this, false);
}
