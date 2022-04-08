// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"

#include "AObjectInSpace.h"

void UMyGameInstance::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if(PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UMyGameInstance, MU))
	{
		UE_LOG(LogTemp, Warning, TEXT("Broadcasting `OnChangedMu`"));
		OnChangedMu.Broadcast(MU);
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
