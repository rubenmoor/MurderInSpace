// Fill out your copyright notice in the Description page of Project Settings.


#include "MyHUDBase.h"

#include "Blueprint/UserWidget.h"
#include "UserWidgetHUDBorder.h"

#define LOCTEXT_NAMESPACE "mynamespace"

AMyHUDBase::AMyHUDBase()
{
	PrimaryActorTick.bCanEverTick = true;

	//HUDGreen = FLinearColor(FVector4d(0.263, 1., 0, 0.7));
	FormattingOptions.SetUseGrouping(false);
	FormattingOptions.SetMinimumFractionalDigits(1);
	FormattingOptions.SetMaximumFractionalDigits(1);
}

void AMyHUDBase::SetWidgetToDefault()
{
	UMGWidget = CreateWidget<UUserWidget>(GetOwningPlayerController(), UMGWidgetDefault);
	UMGWidget->AddToViewport();
	
	UserWidgetHUDBorder = FindOrFail<UUserWidgetHUDBorder>(FName(TEXT("BP_UserWidgetHUDBorder")));
	UserWidgetHUDBorder->SetParams(X0, Y0, X1, Y1);
}

void AMyHUDBase::BeginPlay()
{
	Super::BeginPlay();
	
	if(!UMGWidgetDefault)
	{
		UE_LOG(LogSlate, Error, TEXT("%s: UMGWidgetDefault null"), *GetFullName())
		return;
	}
	
	SetWidgetToDefault();
}

#undef LOCTEXT_NAMESPACE