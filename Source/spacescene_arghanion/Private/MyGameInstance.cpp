// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"

#include "AObjectInSpace.h"
#include "PawnInSpace.h"

void UMyGameInstance::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if(PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UMyGameInstance, MU))
	{
		for(TObjectIterator<AAObjectInSpace> Iter; Iter; ++Iter)
		{
			(*Iter)->UpdateMU(MU);
		}
		for(TObjectIterator<APawnInSpace> Iter; Iter; ++Iter)
		{
			(*Iter)->UpdateMU(MU);
		}
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
