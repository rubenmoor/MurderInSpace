// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"

#include "ActorInSpace.h"
#include "PawnInSpace.h"

UMyGameInstance::UMyGameInstance()
{
	SetUpWorldDimensions(EditorDefaultWorldRadiusMeters, EditorDefaultWorldRadiusUU);
	SetUpCentralBody(EditorDefaultAlpha, EditorDefaultVecF1);
}

void UMyGameInstance::SetUpWorldDimensions(float _WorldRadiusMeters, float _WorldRadius)
{
	WorldRadiusMeters = _WorldRadiusMeters;
	WorldRadius = _WorldRadius;
	ScaleFactor = WorldRadiusMeters / WorldRadius;
	if(ScaleFactor > 10. || ScaleFactor < 0.001)
	{
		UE_LOG(LogTemp, Error, TEXT("UMyGameInstance::SetUpWorldDimensions: ScaleFactor = %f"), ScaleFactor);
		RequestEngineExit(TEXT("ScaleFactor shouldn't be greater than 100.0 or less than 0.01"));
	}
	bSetUpWorldDimensionsDone = true;
}

void UMyGameInstance::SetUpCentralBody(float _Alpha, FVector _VecF1)
{
	if(!bSetUpWorldDimensionsDone)
	{
		UE_LOG(LogTemp, Error, TEXT("UMyGameInstance::SetUpCentralBody"))
		RequestEngineExit(TEXT("Run UMyGameInstance::SetUpWorldDimensions first"));
	}
	Alpha = _Alpha;
	VecF1 = _VecF1;
}

void UMyGameInstance::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const auto Name = PropertyChangedEvent.Property->GetFName();
	static const FName FNameMU = GET_MEMBER_NAME_CHECKED(UMyGameInstance, Alpha);
	if(Name == FNameMU)
	{
		for(TObjectIterator<AOrbit> Iter; Iter; ++Iter)
		{
			(*Iter)->Update(Alpha, WorldRadius, VecF1);
		}
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
