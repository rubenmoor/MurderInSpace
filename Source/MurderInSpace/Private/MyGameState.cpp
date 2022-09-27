// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameState.h"

#include "OrbitComponent.h"

void AMyGameState::BeginPlay()
{
	Super::BeginPlay();
	
	Poisson = std::poisson_distribution<int>(AngularVelocityPoissonMean * 1e3);
	UpdateAllOrbits();
}

void AMyGameState::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
	
	const FName Name = PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName();
	
	static const FName FNameSpaceParams = GET_MEMBER_NAME_CHECKED(AMyGameState, SpaceParams);
	
	if(Name == FNameSpaceParams)
	{
		UpdateAllOrbits();
	}
}

void AMyGameState::UpdateAllOrbits() const
{
	for(TObjectIterator<UOrbitComponent> Iter; Iter; ++Iter)
	{
		(*Iter)->UpdateWithParams(SpaceParams);
	}
}

float AMyGameState::GetInitialAngularVelocity()
{
	return Poisson(RndGen) / 1e3;
}
