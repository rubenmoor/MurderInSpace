// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyPawn_SkeletalMesh.h"
#include "MyCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UNiagaraComponent;

/**
 * The inheritance hierarchy goes like
 *    APawn
 * -> AMyPawn
 * -> AMyPawn_SkeletalMesh
 * -> AMyCharacter
 *
 * Thus not using `ACharacter`
 * 
 */
UCLASS()
class MURDERINSPACE_API AMyCharacter final : public AMyPawn_SkeletalMesh
{
    GENERATED_BODY()

    AMyCharacter();

public:
    UFUNCTION(BlueprintCallable)
    void UpdateSpringArm(uint8 CameraPosition);

    // make stuff visible for the possessing player
    UFUNCTION(BlueprintCallable)
    void ShowEffects();
    
    // hide the astronaut when the camera gets to First-Person view
    UFUNCTION(BlueprintCallable)
    void SetVisibility(bool bVisibility);

    UFUNCTION(BlueprintPure)
    float GetSpringArmLength() const;

    UFUNCTION(BlueprintPure)
    FLinearColor GetTempSplineMeshColor() const { return TempSplineMeshColor; }

    // IHasMesh
    //virtual UPrimitiveComponent* GetMesh() override { return SkeletalMesh; }

protected:

    // event handlers

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor TempSplineMeshColor;
    
    // components

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USpringArmComponent> SpringArm;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UCameraComponent> Camera;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UStaticMeshComponent> Visor;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UStaticMeshComponent> VisorFrame;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USceneComponent> StarAnchor;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UNiagaraComponent> StarsClose;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UNiagaraComponent> StarsDistant;
};
