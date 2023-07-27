#include "HUD/MyCommonButton.h"

#include "Logging/StructuredLog.h"

void UMyCommonButton::NativeOnCurrentTextStyleChanged()
{
	Super::NativeOnCurrentTextStyleChanged();
	TextBlockLabel->SetStyle(GetCurrentTextStyleClass());
}

void UMyCommonButton::NativeConstruct()
{
	Super::NativeConstruct();
	
	OnFocusReceived().AddLambda([this] ()
	{
		PlayAnimation(CaretRotation, 0, 0);
	});
	OnFocusLost().AddLambda([this] ()
	{
		StopAnimation(CaretRotation);
	});
	OnHovered().AddLambda([this] ()
	{
		SetFocus();
	});
	// OnUnhovered().AddLambda([this] ()
	// {
	// 	
	// });
}

void UMyCommonButton::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	if(!TextBlockLabel)
	{
		UE_LOGFMT
			( LogSlate
			, Warning
			, "{0}: UMyCommonButton::SynchronizeProperties: TextBlockLabel null, skipping"
			, GetFName()
			);
		return;
	}
	TextBlockLabel->SetText(Label);
}

// void UMyCommonButton::NativeOnFocusLost(const FFocusEvent& InFocusEvent)
// {
// 	Super::NativeOnFocusLost(InFocusEvent);
// 	UE_LOG(LogSlate, Warning, TEXT("focus lost"))
// 	StopAnimation(CaretRotation);
// }
