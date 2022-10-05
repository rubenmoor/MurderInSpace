// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameState.h"

#include "OrbitComponent.h"

void AMyGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	Poisson = std::poisson_distribution(AngularVelocityPoissonMean * 1e3);
}

void AMyGameState::BeginPlay()
{
	Super::BeginPlay();
	
	UpdateAllOrbits();
}

void AMyGameState::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
	
	const FName Name = PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName();

	// TODO: confirm that this is how it actually works
	static const FName FNameSpaceParams = GET_MEMBER_NAME_CHECKED(AMyGameState, Physics);
	
	if(Name == FNameSpaceParams)
	{
		UpdateAllOrbits();
	}
}

void AMyGameState::UpdateAllOrbits() const
{
	for(TObjectIterator<UOrbitComponent> Iter; Iter; ++Iter)
	{
		(*Iter)->Update(Physics, UStateLib::GetPlayerUIEditorDefault());
	}
}

