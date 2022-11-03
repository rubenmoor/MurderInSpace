// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/MyHUDBase.h"

#include "Actors/CharacterInSpace.h"
#include "Blueprint/UserWidget.h"
#include "HUD/WidgetHUDBorder.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Lib/FunctionLib.h"

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

	LocalPlayerContext = FLocalPlayerContext(GetOwningPlayerController());
	
	UGameInstance* GI = GetGameInstance();
	
	WidgetHUDBorder = CreateWidget<UWidgetHUDBorder>(GI, WidgetHUDBorderClass, FName(TEXT("HUD Border")));
	// TODO: proly have to set foreground color
	WidgetHUDBorder->SetParams(X0, Y0, X1, Y1);
	WidgetHUDBorder->AddToViewport(-1);
}

void AMyHUDBase::HideViewportParentWidgets()
{
	if(IsValid(this))
	{
		TArray<UUserWidget*> ParentWidgets;
		MyObjectIterator<UUserWidget> IWidget([this] (const UUserWidget* Widget)
		{
			return Widget->GetWorld() == GetWorld()
				&& Widget->IsInViewport()
				&& !Widget->IsA(UWidgetHUDBorder::StaticClass());
		});
		for(; IWidget; ++IWidget)
		{
			(*IWidget)->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	else
	{
		UE_LOG(LogSlate, Warning, TEXT("AMyHUDBase: invalid HUD"))
	}
}
