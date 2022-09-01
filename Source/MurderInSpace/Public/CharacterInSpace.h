// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PawnInSpace.h"
#include "Camera/CameraComponent.h"
#include "NiagaraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "CharacterInSpace.generated.h"

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API ACharacterInSpace : public APawnInSpace
{
	GENERATED_BODY()

	ACharacterInSpace();

public:
	UFUNCTION(BlueprintCallable)
	void UpdateSpringArm(uint8 CameraPosition);

	UFUNCTION(BlueprintCallable)
	void SetVisibility(bool bVisibility);

	UFUNCTION(BlueprintPure)
	float GetSpringArmLength() const;

protected:

	// event handers


	// components

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<USkeletalMeshComponent> SkeletalMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> StarAnchor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNiagaraComponent> StarsClose;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNiagaraComponent> StarsDistant;
};