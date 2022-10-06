// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/SMActorInSpace.h"

#include "Actors/GyrationComponent.h"

ASMActorInSpace::ASMActorInSpace()
{
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(MovableRoot);
	Gyration->SetBody(StaticMesh);
	
	StaticMesh->OnBeginCursorOver.AddDynamic(this, &ASMActorInSpace::HandleBeginMouseOver);
	StaticMesh->OnEndCursorOver.AddDynamic(this, &ASMActorInSpace::HandleEndMouseOver);
	StaticMesh->OnClicked.AddDynamic(this, &ASMActorInSpace::HandleClick);
}
