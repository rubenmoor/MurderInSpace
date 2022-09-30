// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyHUDBase.h"
#include "MyHUD.generated.h"

class ACharacterInSpace;
class UImage;
class UOverlay;
class UCanvasPanel;
class UTextBlock;

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API AMyHUD : public AMyHUDBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UTextBlock> TextVelocitySI;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UTextBlock> TextVelocityVCircle;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UTextBlock> TextVelocityDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCanvasPanel> CanvasCenterOfMass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UOverlay> OverlayCenterOfMass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UImage> ImgPointer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<ACharacterInSpace> MyCharacter;

	// event handlers
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
};
