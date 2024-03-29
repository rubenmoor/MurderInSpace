// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyState.h"
#include "GameFramework/PlayerState.h"
#include "MyPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API AMyPlayerState : public APlayerState
{
	GENERATED_BODY()

	friend class UStateLib;
	friend class UMyState;
	
protected:
	// event handlers
	//virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPlayerUI PlayerUI;
	
};
