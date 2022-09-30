// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "GameFramework/HUD.h"

#include "MyHUDBase.generated.h"

class UUserWidgetHUDBorder;

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API AMyHUDBase : public AHUD
{
	GENERATED_BODY()

public:
	AMyHUDBase();

	virtual void SetWidgetToDefault();
	
protected:
	TObjectPtr<UUserWidgetHUDBorder> UserWidgetHUDBorder;
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
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMG Widget Classes")
	TSubclassOf<UUserWidget> UMGWidgetDefault;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UUserWidget> UMGWidget;

	// event handlers
	
	virtual void BeginPlay() override;

	// private methods

	template <typename WidgetT>
	TObjectPtr<WidgetT> FindOrFail(const FName& Name) const
	{
		if(const auto Widget = UMGWidget->WidgetTree.Get()->FindWidget<WidgetT>(Name))
		{
			return Widget;
		}
		else
		{
			UE_LOG(LogSlate, Error, TEXT("%s: FindOrFail: Couldn't find %s"), *GetFullName(), *Name.ToString())
			return nullptr;
		}
	}
	
	FNumberFormattingOptions FormattingOptions;
};
