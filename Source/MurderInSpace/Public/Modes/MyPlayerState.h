// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Lib/UStateLib.h"
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
	
protected:
	// event handlers
	//virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPlayerUI PlayerUI = UStateLib::DefaultPlayerUI;
	
};
