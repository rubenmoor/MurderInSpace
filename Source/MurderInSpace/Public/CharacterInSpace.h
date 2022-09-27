// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PawnInSpace.h"
#include "CharacterInSpace.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UNiagaraComponent;

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

	// hide the astronaut when the camera gets to First-Person view
	UFUNCTION(BlueprintCallable)
	void SetVisibility(bool bVisibility);

	UFUNCTION(BlueprintPure)
	float GetSpringArmLength() const;

	UFUNCTION(BlueprintCallable)
	UOrbitComponent* GetOrbitComponent() { return Orbit; }

protected:

	// event handlers

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
