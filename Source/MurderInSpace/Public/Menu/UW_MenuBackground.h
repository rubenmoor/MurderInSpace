

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_MenuBackground.generated.h"

class UBorder;
/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UUW_MenuBackground : public UUserWidget
{
	GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, meta=(BindWidget))
    TObjectPtr<UBorder> Border;
    
public:
    UBorder* GetBorder() { return Border; }
};
