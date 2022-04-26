// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PawnInSpace.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "CharacterInSpace.generated.h"

/**
 * 
 */
UCLASS()
class SPACESCENE_ARGHANION_API ACharacterInSpace : public APawnInSpace
{
	GENERATED_BODY()

	ACharacterInSpace();

public:
	void UpdateSpringArm(uint8 CameraPosition);
	void SetVisibility(bool bVisibility);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCameraComponent> Camera;
};
