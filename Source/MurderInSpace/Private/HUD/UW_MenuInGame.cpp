#include "HUD/UW_MenuInGame.h"
#include "HUD/MyCommonButton.h"
#include "HUD/MyHUD.h"
#include "Modes/MyGameInstance.h"

void UUW_MenuInGame::NativeConstruct()
{
	Super::NativeConstruct();

	BtnResume->OnClicked().AddLambda([this] ()
	{
		GetPlayerContext().GetHUD<AMyHUD>()->InGameMenuHide();
	});
	BtnSettings->OnClicked().AddLambda([this] ()
	{
		GetPlayerContext().GetHUD<AMyHUDBase>()->SettingsShow();
	});
	BtnLeave->OnClicked().AddLambda([this] ()
	{
		//GetGameInstance<UMyGameInstance>()->LeaveGame(GetPlayerContext());
		GetGameInstance()->ReturnToMainMenu();
	});
	BtnQuit->OnClicked().AddLambda([this] ()
	{
		GetGameInstance<UMyGameInstance>()->QuitGame(GetPlayerContext());
	});
}
