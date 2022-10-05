// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "GameFramework/HUD.h"

#include "MyHUDBase.generated.h"

class UWidgetHUDBorder;

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API AMyHUDBase : public AHUD
{
	GENERATED_BODY()

public:
	AMyHUDBase();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UMG Widget Classes")
	TSubclassOf<UWidgetHUDBorder> WidgetHUDBorderClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UMG Widgets")
	TObjectPtr<UWidgetHUDBorder> WidgetHUDBorder;
	
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
	
	// event handlers
	
	virtual void BeginPlay() override;

	// private methods

	UFUNCTION(BlueprintCallable)
	void HideViewportParentWidgets();

	template <typename WidgetT>
	TObjectPtr<WidgetT> FindOrFail(const TObjectPtr<UUserWidget> InParent, const FName& Name) const
	{
		if(TObjectPtr<WidgetT> Widget = InParent->WidgetTree.Get()->FindWidget<WidgetT>(Name))
		{
			return Widget;
		}
		else
		{
			UE_LOG(LogSlate, Error, TEXT("%s: FindOrFail: Couldn't find %s"), *GetFullName(), *Name.ToString())
			return nullptr;
		}
	}
	
	template <typename WidgetT>
	void WithWidget
		( const TObjectPtr<UUserWidget> InParent
		, const FName& Name
		, const std::function<void(TObjectPtr<WidgetT>)>& Func
		) const
	{
		if(TObjectPtr<WidgetT> Widget = InParent->WidgetTree.Get()->FindWidget<WidgetT>(Name))
		{
			Func(Widget);
		}
		else
		{
			UE_LOG(LogSlate, Error, TEXT("%s: FindOrFail: Couldn't find %s"), *GetFullName(), *Name.ToString())
		}
	}
	
	FNumberFormattingOptions FormattingOptions;
};
