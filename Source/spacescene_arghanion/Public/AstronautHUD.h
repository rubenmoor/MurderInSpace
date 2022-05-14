// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
class SPACESCENE_ARGHANION_API AAstronautHUD : public AHUD
{
	GENERATED_BODY()

	AAstronautHUD();

public:
	TObjectPtr<UCanvasPanel> CanvasPanelMain;
	TObjectPtr<UTextBlock> TextVelocitySI;
	TObjectPtr<UTextBlock> TextVelocityVCircle;
	TObjectPtr<UTextBlock> TextVelocityDirection;

	TObjectPtr<UOverlay> OverlayCenterOfMass;
	TObjectPtr<UImage> ImgPointer;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMGWidget")
	TSubclassOf<class UUserWidget> AssetUMG_AstronautHUD;

	TObjectPtr<UUserWidget> UMG_AstronautHUD;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	template <typename WidgetT>
	TObjectPtr<WidgetT> FindOrFail(const FName& Name) const;
	
	FNumberFormattingOptions FormattingOptions;
};
