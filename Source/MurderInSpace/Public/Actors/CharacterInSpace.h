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
	void DestroyTempSplineMesh();

	UFUNCTION(BlueprintPure)
	USceneComponent* GetTempSplineMeshParent() const { return TempSplineMeshParent; }
	
	UFUNCTION(BlueprintPure)
	FLinearColor GetTempSplineMeshColor() const { return TempSplineMeshColor; }

	/*
	 * update the orbits of all Actors In Space, e.g. asteroids
	 * Note: Client RPC must be method of an actor and only get executed on clients that own said actor
	 * To update all asteroids (NOT owned by any client), the server needs to call a Client RPC with the pawn
	 * that is owned by the client instance.
	 * This is why the `CharacterInSpace` somewhat awkwardly manipulates orbits of actors in space.
	 */
	UFUNCTION(Client, Reliable)
	void ClientRPC_UpdateActorsInSpace(const TArray<FOrbitState>& States, FPhysics Physics, FPlayerUI PlayerUI);
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> TempSplineMeshParent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor TempSplineMeshColor;
};
