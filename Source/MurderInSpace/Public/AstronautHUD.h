// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterInSpace.h"
#include "MyHUDWidget.h"
#include "OrbitDataComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/TextBlock.h"
#include "GameFramework/HUD.h"
#include "AstronautHUD.generated.h"

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API AAstronautHUD : public AHUD
{
	GENERATED_BODY()

	AAstronautHUD();

public:
	TObjectPtr<UCanvasPanel> MainCanvasPanel;
	TObjectPtr<UMyHUDWidget> MyHUDWidget;
	TObjectPtr<UTextBlock> TextVelocitySI;
	TObjectPtr<UTextBlock> TextVelocityVCircle;
	TObjectPtr<UTextBlock> TextVelocityDirection;

	TObjectPtr<UCanvasPanel> CanvasCenterOfMass;
	TObjectPtr<UOverlay> OverlayCenterOfMass;
	TObjectPtr<UTextBlock> TextCenterOfMass;
	TObjectPtr<UImage> ImgPointer;

	TObjectPtr<UImage> ImgDebug;

	// relative to viewport width: the distance from the HUD to the viewport edge at top and bottom
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float X0 = .11;
	
	// relative to viewport height: the distance from the HUD to the viewport edge at top and bottom
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Y0 = .1;

	// green overlay for the HUD
	//FLinearColor HUDGreen;
	
	// parameter for the cubic bezier that draws the limits of the HUD
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float X1 = .2;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Y1 = .2;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMGWidget")
	TSubclassOf<class UUserWidget> AssetUMG_AstronautHUD;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UUserWidget> UMG_AstronautHUD;

	// event handlers
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	TObjectPtr<ACharacterInSpace> Pawn;
	
	template <typename WidgetT>
	TObjectPtr<WidgetT> FindOrFail(const FName& Name) const;
	
	FNumberFormattingOptions FormattingOptions;
};
