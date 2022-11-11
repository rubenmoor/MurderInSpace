// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/MyActor_StaticMesh.h"

#include "Actors/GyrationComponent.h"
#include "Modes/MyGameInstance.h"

AMyActor_StaticMesh::AMyActor_StaticMesh()
{
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(MovableRoot);
	
    Gyration = CreateDefaultSubobject<UGyrationComponent>(TEXT("Gyration"));
}

void AMyActor_StaticMesh::BeginPlay()
{
	Super::BeginPlay();

	UMyGameInstance* GI = GetGameInstance<UMyGameInstance>();
	
	OnBeginCursorOver.AddDynamic(GI, &UMyGameInstance::HandleBeginMouseOver);
	OnEndCursorOver.AddDynamic(GI, &UMyGameInstance::HandleEndMouseOver);
	OnClicked.AddDynamic(GI, &UMyGameInstance::HandleClick);
}
