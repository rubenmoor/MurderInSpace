


#include "Modes/MyAssetManager.h"

#include "AbilitySystemGlobals.h"

void UMyAssetManager::StartInitialLoading()
{
    Super::StartInitialLoading();
    // https://github.com/tranek/GASDocumentation#concepts-asg-initglobaldata
    UAbilitySystemGlobals::Get().InitGlobalData();
}
