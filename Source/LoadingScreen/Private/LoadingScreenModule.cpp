#include "LoadingScreenModule.h"

#include "MoviePlayer.h"
#include "Modules/ModuleManager.h"

void LoadingScreenModule::StartIngameLoadingScreen(bool bPlayUntilStopped, float Playtime)
{
    FLoadingScreenAttributes Attrs;
    Attrs.bAutoCompleteWhenLoadingCompletes = !bPlayUntilStopped;
    Attrs.bWaitForManualStop = bPlayUntilStopped;
    Attrs.bAllowEngineTick = bPlayUntilStopped;
    Attrs.MinimumLoadingScreenDisplayTime = 10;
    Attrs.WidgetLoadingScreen = FLoadingScreenAttributes::NewTestLoadingScreenWidget();
    GetMoviePlayer()->SetupLoadingScreen(Attrs);
}

void LoadingScreenModule::StopIngameLoadingScreen()
{
    GetMoviePlayer()->StopMovie();
}

IMPLEMENT_MODULE(LoadingScreenModule, LoadingScreen);
