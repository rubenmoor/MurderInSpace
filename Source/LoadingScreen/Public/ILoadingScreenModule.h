#pragma once

#include "Modules/ModuleManager.h"

class ILoadingScreenModule : public IModuleInterface
{
public:
    static ILoadingScreenModule& Get()
    {
        return FModuleManager::LoadModuleChecked<ILoadingScreenModule>("LoadingScreen");
    }
    virtual void StartIngameLoadingScreen(bool bPlayUntilStopped, float Playtime) = 0;
    virtual void StopIngameLoadingScreen() = 0;
};