#include "HUD/UW_MyAbilities.h"

#include "CommonTextBlock.h"
#include "MyGameplayTags.h"
#include "Components/Image.h"
#include "Components/Border.h"

void UUW_MyAbilities::SetVisibilityArrow(FGameplayTag InTag, bool InBVisible)
{
    const auto NewVisibility = InBVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden;
    const auto& Tag = FMyGameplayTags::Get();
    check(InTag.MatchesTag(Tag.Ability))

    const TObjectPtr<UCommonTextBlock> TextBlock =
          InTag == Tag.AbilityAccelerate
        ? TextAccActive
        : InTag == Tag.AbilityEmbrace
        ? TextGrpActive
        : InTag == Tag.AbilityKick
        ? TextPshActive
        : InTag == Tag.AbilityMoveTowardsCircle
        ? TextCrcActive
        : InTag == Tag.AbilityRecover
        ? TextRecActive
        : InTag == Tag.AbilityLookAt
        ? TextLktActive
        : nullptr;

    check(IsValid(TextBlock))
    TextBlock->SetVisibility(NewVisibility);
}

void UUW_MyAbilities::SetFilled(const FGameplayTagContainer& InTags, bool InBFilled)
{
    const auto& Tag = FMyGameplayTags::Get();
    check(!InTags.Filter(Tag.Ability.GetSingleTagContainer()).IsEmpty())

    const auto NewVisibilityFilled = InBFilled ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden;
    const auto NewVisibility = InBFilled ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible;

    if(InTags.HasTag(Tag.AbilityAccelerate))
    {
        ImgAccFilled->SetVisibility(NewVisibilityFilled);
        ImgAcc->SetVisibility(NewVisibility);
    }
    else if(InTags.HasTag(Tag.AbilityEmbrace))
    {
        ImgGrpFilled->SetVisibility(NewVisibilityFilled);
        ImgGrp->SetVisibility(NewVisibility);
    }
    else if(InTags.HasTag(Tag.AbilityKick))
    {
        ImgPshFilled->SetVisibility(NewVisibilityFilled);
        ImgPsh->SetVisibility(NewVisibility);
    }
    else if(InTags.HasTag(Tag.AbilityMoveTowardsCircle))
    {
        ImgCrcFilled->SetVisibility(NewVisibilityFilled);
        ImgCrc->SetVisibility(NewVisibility);
    }
    else if(InTags.HasTag(Tag.AbilityRecover))
    {
        ImgRecFilled->SetVisibility(NewVisibilityFilled);
        ImgRec->SetVisibility(NewVisibility);
    }
    else if(InTags.HasTag(Tag.AbilityLookAt))
    {
        ImgLktFilled->SetVisibility(NewVisibilityFilled);
        ImgLkt->SetVisibility(NewVisibility);
    }
}

void UUW_MyAbilities::SetBordered(FGameplayTag InTag, bool InBBordered)
{
    const auto NewVisibility = InBBordered ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden;
    const auto& Tag = FMyGameplayTags::Get();
    check(InTag.MatchesTag(Tag.Ability))

    const TObjectPtr<UBorder> Border =
          InTag == Tag.AbilityAccelerate
        ? BorderAcc
        : InTag == Tag.AbilityEmbrace
        ? BorderGrp
        : InTag == Tag.AbilityKick
        ? BorderPsh
        : InTag == Tag.AbilityMoveTowardsCircle
        ? BorderCrc
        : InTag == Tag.AbilityRecover
        ? BorderRec
        : InTag == Tag.AbilityLookAt
        ? BorderLkt
        : nullptr;

    check(IsValid(Border))
    Border->SetVisibility(NewVisibility);
}
