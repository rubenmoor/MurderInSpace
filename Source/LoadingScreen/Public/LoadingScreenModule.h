#pragma once
#include "ILoadingScreenModule.h"

class LoadingScreenImpl : public ILoadingScreenModule
{
    virtual void StartIngameLoadingScreen(bool bPlayUntilStopped, float Playtime) override;
    virtual void StopIngameLoadingScreen() override;
};

