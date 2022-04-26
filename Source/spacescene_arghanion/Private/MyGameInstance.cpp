// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"

#include "AObjectInSpace.h"
#include "PawnInSpace.h"

void UMyGameInstance::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const auto Name = PropertyChangedEvent.Property->GetFName();
	static const FName FNameMU = GET_MEMBER_NAME_CHECKED(UMyGameInstance, MU);
	static const FName FNameGameAreaRadius = GET_MEMBER_NAME_CHECKED(UMyGameInstance, GameAreaRadius);
	if(Name == FNameMU || Name == FNameGameAreaRadius)
	{
		for(TObjectIterator<AAObjectInSpace> Iter; Iter; ++Iter)
		{
			(*Iter)->UpdateMU(MU, GameAreaRadius);
		}
		for(TObjectIterator<APawnInSpace> Iter; Iter; ++Iter)
		{
			(*Iter)->UpdateMU(MU, GameAreaRadius);
		}
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
