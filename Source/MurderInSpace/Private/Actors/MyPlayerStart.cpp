// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/MyPlayerStart.h"

#include "Orbit/Orbit.h"
#include "Net/UnrealNetwork.h"

AMyPlayerStart::AMyPlayerStart()
{
	bNetLoadOnClient = false;
	bReplicates = true;
	// TODO: not sure if necessary, but not harmful either
    bAlwaysRelevant = true;
	AActor::SetReplicateMovement(false);
}

void AMyPlayerStart::Destroyed()
{
	Super::Destroyed();
	if(IsValid(RP_Orbit))
	{
		RP_Orbit->Destroy();
	}
}

void AMyPlayerStart::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
    if  (
		// avoid orbit spawning when editing and compiling blueprint
		   GetWorld()->WorldType != EWorldType::EditorPreview
		
		// avoid orbit spawning when dragging an actor with orbit into the viewport at first
		// The preview actor that is created doesn't have a valid location
		// Once the actor is placed inside the viewport, it's no longer transient and the orbit is reconstructed properly
		// according to the actor location
		&& !HasAnyFlags(RF_Transient)
		)
    {
		OrbitSetup(this);
    }
}

void AMyPlayerStart::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(AMyPlayerStart, RP_Orbit      , COND_InitialOnly)
}
