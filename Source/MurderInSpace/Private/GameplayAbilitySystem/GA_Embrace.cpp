// 


#include "GameplayAbilitySystem/GA_Embrace.h"

#include "MyGameplayTags.h"

UGA_Embrace::UGA_Embrace()
{
    const auto& Tag = FMyGameplayTags::Get();
    AbilityTags.AddTag(Tag.InputBindingAbilityEmbrace);
    ActivationOwnedTags.AddTag(Tag.Embrace);
}
