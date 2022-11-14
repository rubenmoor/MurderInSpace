// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/MyPlayerStart.h"

#include "Actors/Orbit.h"

void AMyPlayerStart::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if(HasAnyFlags(RF_ClassDefaultObject | RF_Transient))
	{
		return;
	}
	
	if(!OrbitClass)
	{
		UE_LOG(LogMyGame, Error, TEXT("%s: OnConstruction: OrbitClass null"), *GetFullName())
		return;
	}
	const FString NewName = FString(TEXT("Orbit_")).Append(GetFName().ToString());
	if(!IsValid(Orbit))
	{
		Orbit = AOrbit::SpawnOrbit(this, NewName);
	}
	else
	{
		// fix orbit after copying (alt + move in editor)
		if(Orbit->GetBody() != this)
		{
			UObject* Object = StaticFindObjectFastSafe(AOrbit::StaticClass(), GetWorld(), FName(NewName), true);
			Orbit = Object ? Cast<AOrbit>(Object) : nullptr;
		}
		Orbit->OnConstruction(FTransform());
	}
}
