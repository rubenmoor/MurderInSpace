#pragma once
#include "ILoadingScreenModule.h"

class LOADINGSCREEN_API LoadingScreenModule : public ILoadingScreenModule
{
public:
    virtual void StartIngameLoadingScreen(bool bPlayUntilStopped, float Playtime) override;
    virtual void StopIngameLoadingScreen() override;
};

