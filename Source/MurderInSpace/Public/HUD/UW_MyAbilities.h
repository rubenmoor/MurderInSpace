#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_MyAbilities.generated.h"

struct FGameplayTagContainer;
class UBorder;
class UImage;
class UCommonTextBlock;
class UVerticalBox;
/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UUW_MyAbilities : public UUserWidget
{
    GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetVisibilityArrow(FGameplayTag InTag, bool InBVisible);

	UFUNCTION(BlueprintCallable)
	void SetFilled(const FGameplayTagContainer& InTags, bool InBFilled);

	UFUNCTION(BlueprintCallable)
	void SetBordered(FGameplayTag InTag, bool InBBordered);
	
protected:
	// acc, acceleration ability
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> VerticalBoxAcc;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> TextAccActive;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ImgAccFilled;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UBorder> BorderAcc;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ImgAcc;

	// grp, grab, embrace ability

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> VerticalBoxGrp;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> TextGrpActive;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ImgGrpFilled;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UBorder> BorderGrp;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ImgGrp;

	// psh, push, kick ability
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> VerticalBoxPsh;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> TextPshActive;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ImgPshFilled;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UBorder> BorderPsh;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ImgPsh;

	// crc, circle, move towards circle ability
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> VerticalBoxCrc;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> TextCrcActive;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ImgCrcFilled;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UBorder> BorderCrc;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ImgCrc;

	// rec, recover ability
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> VerticalBoxRec;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> TextRecActive;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ImgRecFilled;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UBorder> BorderRec;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ImgRec;
	
	// lkt, look at ability
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> VerticalBoxLkt;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> TextLktActive;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ImgLktFilled;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UBorder> BorderLkt;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ImgLkt;
};
