// 


#include "GameplayAbilitySystem/GA_Kick.h"

#include "MyGameplayTags.h"

UGA_Kick::UGA_Kick()
{
    const auto& Tag = FMyGameplayTags::Get();
    AbilityTags.AddTag(Tag.InputBindingAbilityKick);
}
