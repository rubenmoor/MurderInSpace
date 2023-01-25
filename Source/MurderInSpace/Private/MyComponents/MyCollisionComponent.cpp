// Fill out your copyright notice in the Description page of Project Settings.


#include "MyComponents/MyCollisionComponent.h"

#include "Modes/MyState.h"

UMyCollisionComponent::UMyCollisionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMyCollisionComponent::HandleHit(const FHitResult& HitResult)
{
	UE_LOG(LogMyGame, Warning, TEXT("Hit!"))
	// TODO: only one actor needs to perform the calculations;
	// lets define a primary and a secondary actor
	// use a co-routine to await the result from the primary actor, maybe?
}
