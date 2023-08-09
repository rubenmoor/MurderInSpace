#include "MyGameplayTags.h"

#include "GameplayTagsManager.h"

DEFINE_LOG_CATEGORY(LogMyGameplayTags);

FMyGameplayTags::FMyGameplayTags()
{
	auto& GTM = UGameplayTagsManager::Get();
	Acceleration                   = GTM.AddNativeGameplayTag("Acceleration");
	AccelerationTranslational      = GTM.AddNativeGameplayTag("Acceleration.Translational");
	AccelerationRotational         = GTM.AddNativeGameplayTag("Acceleration.Rotational");
	AccelerationRotationalCCW      = GTM.AddNativeGameplayTag("Acceleration.Rotational.CCW");
	AccelerationRotationalCW       = GTM.AddNativeGameplayTag("Acceleration.Rotational.CW");

    AccelerationMoveTowardsCircle  = GTM.AddNativeGameplayTag("Acceleration.MoveTowardsCircle");
	
	InputBindingAbility                  = GTM.AddNativeGameplayTag("InputBinding.Ability");
	InputBindingAbilityAccelerate        = GTM.AddNativeGameplayTag("InputBinding.Ability.Accelerate");
	InputBindingAbilityMoveTowardsCircle = GTM.AddNativeGameplayTag("InputBinding.Ability.MoveTowardsCircle");
	InputBindingAbilityEmbrace           = GTM.AddNativeGameplayTag("InputBinding.Ability.Embrace");
	InputBindingAbilityKick              = GTM.AddNativeGameplayTag("InputBinding.Ability.Kick");

    Ability           = GTM.AddNativeGameplayTag("Ability");
    AbilityAccelerate = GTM.AddNativeGameplayTag("Ability.Accelerate");
    AbilityLookAt     = GTM.AddNativeGameplayTag("Ability.LookAt");
	AbilityEmbrace    = GTM.AddNativeGameplayTag("Ability.Embrace");
	AbilityKick       = GTM.AddNativeGameplayTag("Ability.Kick");

	GameplayCue                = GTM.AddNativeGameplayTag("GameplayCue");
	
	CuePose             = GTM.AddNativeGameplayTag("GameplayCue.Pose");
	CuePoseAccelerate   = GTM.AddNativeGameplayTag("GameplayCue.Pose.Accelerate");
	CuePoseTurn         = GTM.AddNativeGameplayTag("GameplayCue.Pose.Turn");
	CuePoseTurnCCW      = GTM.AddNativeGameplayTag("GameplayCue.Pose.Turn.CCW");
	CuePoseTurnCW       = GTM.AddNativeGameplayTag("GameplayCue.Pose.Turn.CW");
	CuePoseEmbrace      = GTM.AddNativeGameplayTag("GameplayCue.Pose.Embrace"      );
	CuePoseKickPosition = GTM.AddNativeGameplayTag("GameplayCue.Pose.Kick.Position");
	CuePoseKickExecute  = GTM.AddNativeGameplayTag("GameplayCue.Pose.Kick.Execute" );

	CueAccelerateShowThrusters = GTM.AddNativeGameplayTag("GameplayCue.Accelerate.ShowThrusters");
	CueAccelerateFire          = GTM.AddNativeGameplayTag("GameplayCue.Accelerate.Fire");
    GameplayCueLocal           = GTM.AddNativeGameplayTag("GameplayCue.Local");
	LocalCueAccelerateFire     = GTM.AddNativeGameplayTag("GameplayCue.Local.Accelerate.Fire");
	
	InputBindingCustom                  = GTM.AddNativeGameplayTag("InputBinding.Custom");
	InputBindingCustomZoom              = GTM.AddNativeGameplayTag("InputBinding.Custom.Zoom");
	InputBindingCustomSelect            = GTM.AddNativeGameplayTag("InputBinding.Custom.Select");
	InputBindingCustomAllOrbitsShowHide = GTM.AddNativeGameplayTag("InputBinding.Custom.AllOrbits.ShowHide");
	InputBindingCustomMyOrbitShowHide   = GTM.AddNativeGameplayTag("InputBinding.Custom.MyOrbit.ShowHide");
	InputBindingCustomIngameMenuToggle  = GTM.AddNativeGameplayTag("InputBinding.Custom.IngameMenu.Toggle");
}

const FMyGameplayTags FMyGameplayTags::Singleton;

IMPLEMENT_MODULE(GameplayTagsModule, MyGameplayTags)
