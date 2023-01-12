// Fill out your copyright notice in the Description page of Project Settings.


#include "Modes/MyGameState.h"

#include "Actors/Orbit.h"
#include "Lib/FunctionLib.h"
#include "Net/UnrealNetwork.h"

void AMyGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	Poisson = std::poisson_distribution(AngularVelocityPoissonMean * 1e3);
}

void AMyGameState::OnRep_Physics()
{
    UMyState* MyState = GEngine->GetEngineSubsystem<UMyState>();
	UWorld* World = GetWorld();
	auto Filter = [this, World] (const AOrbit* Orbit)
	{
		return Orbit->GetWorld() == World
			&& Orbit->GetIsInitialized();
	};
	for(TMyObjectIterator<AOrbit> IOrbit(Filter); IOrbit; ++IOrbit)
	{
		MyState->WithInstanceUI(this, [this, MyState, IOrbit] (FInstanceUI& InstanceUI)
		{
			(*IOrbit)->Update(FVector::Zero(), MyState->GetPhysics(this), InstanceUI);
		});
	}
}

#if WITH_EDITOR
void AMyGameState::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
	
	const FName Name = PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName();

	// TODO: confirm that this is how it actually works
	static const FName FNamePhysics = GET_MEMBER_NAME_CHECKED(AMyGameState, RP_Physics);
	
	if(Name == FNamePhysics)
	{
		for(TMyObjectIterator<AOrbit> IOrbit; IOrbit; ++IOrbit)
		{
			//(*IOrbit)->SetInitialParams(RP_Physics);
			(*IOrbit)->Update(FVector::Zero(), RP_Physics, InstanceUIEditorDefault);
		}
	}
}
#endif

void AMyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMyGameState, RP_Physics)
}