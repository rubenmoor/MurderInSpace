// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"

#include "PawnInSpace.h"
#include "UObject/UObjectIterator.h"

UMyGameInstance::UMyGameInstance()
{
	Random.GenerateNewSeed();
}
