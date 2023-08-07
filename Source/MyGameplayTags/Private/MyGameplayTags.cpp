#include "MyGameplayTags.h"

#include "GameplayTagsManager.h"

DEFINE_LOG_CATEGORY(LogMyGameplayTags);

FMyGameplayTags::FMyGameplayTags()
{
	auto& GTM = UGameplayTagsManager::Get();
	Acceleration                   = GTM.AddNativeGameplayTag("Acceleration");
	AccelerationTranslational      = GTM.AddNativeGameplayTag("Acceleration.Translational");
	AccelerationRotational         = GTM.AddNativeGameplayTag("Acceleration.Rotational");
	AccelerationRotationalDuration = GTM.AddNativeGameplayTag("Acceleration.Rotational.Duration");
	AccelerationRotationalCCW      = GTM.AddNativeGameplayTag("Acceleration.Rotational.CCW");
	AccelerationRotationalCW       = GTM.AddNativeGameplayTag("Acceleration.Rotational.CW");

    AccelerationMoveTowardsCircle  = GTM.AddNativeGameplayTag("Acceleration.MoveTowardsCircle");
	
	InputBindingAbility                  = GTM.AddNativeGameplayTag("InputBinding.Ability");
	InputBindingAbilityAccelerate        = GTM.AddNativeGameplayTag("InputBinding.Ability.Accelerate");
	InputBindingAbilityLookAt            = GTM.AddNativeGameplayTag("InputBinding.Ability.LookAt");
	InputBindingAbilityMoveTowardsCircle = GTM.AddNativeGameplayTag("InputBinding.Ability.MoveTowardsCircle");
	InputBindingAbilityEmbrace           = GTM.AddNativeGameplayTag("InputBinding.Ability.Embrace");
	InputBindingAbilityKick              = GTM.AddNativeGameplayTag("InputBinding.Ability.Kick");

    Ability           = GTM.AddNativeGameplayTag("Ability");
    AbilityAccelerate = GTM.AddNativeGameplayTag("Ability.Accelerate");
    AbilityLookAt     = GTM.AddNativeGameplayTag("Ability.LookAt");
	AbilityEmbrace    = GTM.AddNativeGameplayTag("Ability.Embrace");
	AbilityKick       = GTM.AddNativeGameplayTag("Ability.Kick");

	Pose             = GTM.AddNativeGameplayTag("Pose");
	PoseAccelerate   = GTM.AddNativeGameplayTag("Pose.Accelerate");
	PoseTurnCCW      = GTM.AddNativeGameplayTag("Pose.Turn.CCW");
	PoseTurnCW       = GTM.AddNativeGameplayTag("Pose.Turn.CW");
	PoseEmbrace      = GTM.AddNativeGameplayTag("Pose.Embrace"      );
	PoseKickPosition = GTM.AddNativeGameplayTag("Pose.Kick.Position");
	PoseKickExecute  = GTM.AddNativeGameplayTag("Pose.Kick.Execute" );

	
	GameplayCue                = GTM.AddNativeGameplayTag("GameplayCue");
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
