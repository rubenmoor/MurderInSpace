#include "MyGameplayTags.h"

#include "GameplayTagsManager.h"

DEFINE_LOG_CATEGORY(LogMyGameplayTags);

FMyGameplayTags::FMyGameplayTags()
{
	auto& GTM = UGameplayTagsManager::Get();

	Ability                  = GTM.AddNativeGameplayTag("Ability");
	AbilityAccelerate        = GTM.AddNativeGameplayTag("Ability.Accelerate");
	AbilityMoveTowardsCircle = GTM.AddNativeGameplayTag("Ability.MoveTowardsCircle");
	AbilityEmbrace           = GTM.AddNativeGameplayTag("Ability.Embrace");
	AbilityKick              = GTM.AddNativeGameplayTag("Ability.Kick");
    AbilityLookAt            = GTM.AddNativeGameplayTag("Ability.LookAt");
	AbilityRecover           = GTM.AddNativeGameplayTag("Ability.Recover");

	BlockingTurn = GTM.AddNativeGameplayTag("BlockingTurn");

	GameplayCue         = GTM.AddNativeGameplayTag("GameplayCue");
	
	CuePose             = GTM.AddNativeGameplayTag("GameplayCue.Pose");
	CuePoseAccelerate   = GTM.AddNativeGameplayTag("GameplayCue.Pose.Accelerate");
	CuePoseTorque       = GTM.AddNativeGameplayTag("GameplayCue.Pose.Torque");
	CuePoseTorqueCCW    = GTM.AddNativeGameplayTag("GameplayCue.Pose.Torque.CCW");
	CuePoseTorqueCW     = GTM.AddNativeGameplayTag("GameplayCue.Pose.Torque.CW");
	CuePoseEmbrace      = GTM.AddNativeGameplayTag("GameplayCue.Pose.Embrace"      );
	CuePoseKickPosition = GTM.AddNativeGameplayTag("GameplayCue.Pose.Kick.Position");
	CuePoseKickExecute  = GTM.AddNativeGameplayTag("GameplayCue.Pose.Kick.Execute" );

	CueShowThrusters = GTM.AddNativeGameplayTag("GameplayCue.ShowThrusters");
	CueThrustersFire = GTM.AddNativeGameplayTag("GameplayCue.ThrustersFire");
	
    GameplayCueLocal    = GTM.AddNativeGameplayTag("GameplayCue.Local");
	
	CustomInputBinding                  = GTM.AddNativeGameplayTag("CustomInputBinding");
	CustomInputBindingZoom              = GTM.AddNativeGameplayTag("CustomInputBinding.Zoom");
	CustomInputBindingSelect            = GTM.AddNativeGameplayTag("CustomInputBinding.Select");
	CustomInputBindingAllOrbitsShowHide = GTM.AddNativeGameplayTag("CustomInputBinding.AllOrbits.ShowHide");
	CustomInputBindingMyOrbitShowHide   = GTM.AddNativeGameplayTag("CustomInputBinding.MyOrbit.ShowHide");
	CustomInputBindingIngameMenuToggle  = GTM.AddNativeGameplayTag("CustomInputBinding.IngameMenu.Toggle");
}

const FMyGameplayTags FMyGameplayTags::Singleton;

IMPLEMENT_MODULE(GameplayTagsModule, MyGameplayTags)
