// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AstronautHUD.generated.h"

class ACharacterInSpace;
class UImage;
class UOverlay;
class UCanvasPanel;
class UTextBlock;
class UUserWidgetHUDBorder;

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API AAstronautHUD : public AHUD
{
	GENERATED_BODY()

	AAstronautHUD();

public:
	//TObjectPtr<UCanvasPanel> MainCanvasPanel;
	TObjectPtr<UUserWidgetHUDBorder> UserWidgetHUDBorder;
	//TObjectPtr<UMyHUDWidget> MyHUDWidget;
	TObjectPtr<UTextBlock> TextVelocitySI;
	TObjectPtr<UTextBlock> TextVelocityVCircle;
	TObjectPtr<UTextBlock> TextVelocityDirection;

	TObjectPtr<UCanvasPanel> CanvasCenterOfMass;
	TObjectPtr<UOverlay> OverlayCenterOfMass;
	TObjectPtr<UImage> ImgPointer;

    // Default values: overridden by blueprint
	// relative to viewport width: the horizontal distance from the circular HUD
	// to the viewport edge at top and bottom
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float X0 = 0.115;
	
	// relative to viewport height: the distance from the HUD to the viewport edge
	// at top and bottom
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Y0 = 0.0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float X1 = -0.02;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Y1 = 0.33;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMGWidget")
	TSubclassOf<class UUserWidget> AssetUMG_AstronautHUD;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UUserWidget> UMG_AstronautHUD;

	// event handlers
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	TObjectPtr<ACharacterInSpace> MyCharacter;
	
	template <typename WidgetT>
	TObjectPtr<WidgetT> FindOrFail(const FName& Name) const;
	
	FNumberFormattingOptions FormattingOptions;
};
