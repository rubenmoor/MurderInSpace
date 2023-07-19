#include "GameplayAbilitySystem//MyGameplayTags.h"

#include "GameplayTagsManager.h"
#include "Modes/MyPlayerController.h"

FMyGameplayTags::FMyGameplayTags()
{
	UGameplayTagsManager::OnLastChanceToAddNativeTags().AddLambda([this] ()
	{
		auto& GTM = UGameplayTagsManager::Get();

		{
			using enum EInputAction;
			InputActionTags.SetNumZeroed(static_cast<uint8>(Last) + 1);
		
			InputActionTags[static_cast<uint8>(AccelerateBeginEnd     )] = GTM.AddNativeGameplayTag("InputAction.AccelerateBeginEnd");
			InputActionTags[static_cast<uint8>(TowardsCircleBeginEnd  )] = GTM.AddNativeGameplayTag("InputAction.TowardsCircleBeginEnd");
			InputActionTags[static_cast<uint8>(EmbraceBeginEnd        )] = GTM.AddNativeGameplayTag("InputAction.EmbraceBeginEnd");
			InputActionTags[static_cast<uint8>(KickPositionExecute    )] = GTM.AddNativeGameplayTag("InputAction.KickPositionExecute");
			InputActionTags[static_cast<uint8>(KickCancel             )] = GTM.AddNativeGameplayTag("InputAction.KickPositionExecutCancel");
			
			InputActionTags[static_cast<uint8>(IngameMenuToggle       )] = GTM.AddNativeGameplayTag("InputAction.IngameMenuToggle");
			InputActionTags[static_cast<uint8>(MyTrajectoryShowHide   )] = GTM.AddNativeGameplayTag("InputAction.MyTrajectoryShowHide");
			InputActionTags[static_cast<uint8>(AllTrajectoriesShowHide)] = GTM.AddNativeGameplayTag("InputAction.AllTrajectoriesShowHide");
			InputActionTags[static_cast<uint8>(MyTrajectoryToggle     )] = GTM.AddNativeGameplayTag("InputAction.MyTrajectoryToggle");
			InputActionTags[static_cast<uint8>(Zoom                   )] = GTM.AddNativeGameplayTag("InputAction.Zoom"            );
			InputActionTags[static_cast<uint8>(Select                 )] = GTM.AddNativeGameplayTag("InputAction.Select"          );
		}
		IsAccelerating = GTM.AddNativeGameplayTag("AttrSetAcceleration.IsAccelerating");
		Accelerate = GTM.AddNativeGameplayTag("Ability.Accelerate");
		IsMovingTowardsCircle = GTM.AddNativeGameplayTag("AttrSetAcceleration.IsMovingTowardsCircle");
	});
}

const FGameplayTag& FMyGameplayTags::GetInputActionTag(EInputAction InputAction) const
{
    return InputActionTags[static_cast<uint8>(InputAction)];
}

FMyGameplayTags FMyGameplayTags::MyGameplayTags;