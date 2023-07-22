#include "GameplayAbilitySystem/MyGameplayAbility.h"

#include "GameplayAbilitySystem/MyDeveloperSettings.h"

void UMyGameplayAbility::BindOnRelease(std::function<void()> Callback)
{
    OnReleaseDelegate.BindLambda([=]
    {
        Callback();
        OnReleaseDelegate.Unbind();
    });
}
