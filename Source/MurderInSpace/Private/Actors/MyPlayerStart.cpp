// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/MyPlayerStart.h"

#include "Actors/OrbitComponent.h"

AMyPlayerStart::AMyPlayerStart()
{
	GetOrbit()->SetEnableVisibility(false);
}
