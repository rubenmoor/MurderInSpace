// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MyGameState.h"

#include "Actors/OrbitComponent.h"
#include "Lib/FunctionLib.h"

void AMyGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	Poisson = std::poisson_distribution(AngularVelocityPoissonMean * 1e3);
}

#if WITH_EDITOR
void AMyGameState::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
	
	const FName Name = PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName();

	// TODO: confirm that this is how it actually works
	static const FName FNameSpaceParams = GET_MEMBER_NAME_CHECKED(AMyGameState, Physics);
	
	if(Name == FNameSpaceParams)
	{
		for(TObjectIterator<UOrbitComponent> IOrbit; IOrbit; ++IOrbit)
		{
			(*IOrbit)->Update(Physics, UStateLib::GetInstanceUIEditorDefault());
		}
	}
}
#endif

