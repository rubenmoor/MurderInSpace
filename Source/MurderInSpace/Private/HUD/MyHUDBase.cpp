// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/MyHUDBase.h"

#include "Blueprint/UserWidget.h"
#include "HUD/WidgetHUDBorder.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

#define LOCTEXT_NAMESPACE "mynamespace"

AMyHUDBase::AMyHUDBase()
{
	PrimaryActorTick.bCanEverTick = true;

	FormattingOptions.SetUseGrouping(false);
	FormattingOptions.SetMinimumFractionalDigits(1);
	FormattingOptions.SetMaximumFractionalDigits(1);

	// TODO: find a way to set HUD default style
}

void AMyHUDBase::BeginPlay()
{
	Super::BeginPlay();
	
	WidgetHUDBorder = CreateWidget<UWidgetHUDBorder>(GetOwningPlayerController(), WidgetHUDBorderClass);
	WidgetHUDBorder->SetParams(X0, Y0, X1, Y1);
	WidgetHUDBorder->AddToViewport();
}

void AMyHUDBase::HideViewportParentWidgets()
{
	TArray<UUserWidget*> ParentWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), ParentWidgets, UUserWidget::StaticClass());
	for(const auto Widget : ParentWidgets)
	{
		if(!Widget->IsA(WidgetHUDBorder->StaticClass()))
		{
			Widget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	
}

#undef LOCTEXT_NAMESPACE
