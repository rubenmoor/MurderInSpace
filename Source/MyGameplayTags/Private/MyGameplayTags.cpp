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

	Embrace      = GTM.AddNativeGameplayTag("Embrace"      );
	KickPosition = GTM.AddNativeGameplayTag("Kick.Position");
	KickExecute  = GTM.AddNativeGameplayTag("Kick.Execute" );

    AccelerationMoveTowardsCircle  = GTM.AddNativeGameplayTag("Acceleration.MoveTowardsCircle");
	
	InputBindingAbility                  = GTM.AddNativeGameplayTag("InputBinding.Ability");
	InputBindingAbilityAccelerate        = GTM.AddNativeGameplayTag("InputBinding.Ability.Accelerate");
	InputBindingAbilityLookAt            = GTM.AddNativeGameplayTag("InputBinding.Ability.LookAt");
	InputBindingAbilityMoveTowardsCircle = GTM.AddNativeGameplayTag("InputBinding.Ability.MoveTowardsCircle");
	InputBindingAbilityEmbrace           = GTM.AddNativeGameplayTag("InputBinding.Ability.Embrace");
	InputBindingAbilityKick              = GTM.AddNativeGameplayTag("InputBinding.Ability.Kick");

    Ability       = GTM.AddNativeGameplayTag("Ability");
    AbilityLookAt = GTM.AddNativeGameplayTag("Ability.LookAt");
	
	GiveInitiallyToMyPawn    = GTM.AddNativeGameplayTag("GiveTo.Initial.MyPawn");

	// still not using those
	GameplayCue              = GTM.AddNativeGameplayTag("GameplayCue");
    GameplayCueLocal         = GTM.AddNativeGameplayTag("GameplayCue.Local");
    LocalShowMyOrbit         = GTM.AddNativeGameplayTag("GameplayCue.Local.ShowMyOrbit");
    LocalShowMySnapshotOrbit = GTM.AddNativeGameplayTag("GameplayCue.Local.ShowMySnapshotOrbit");
	
	InputBindingCustom                  = GTM.AddNativeGameplayTag("InputBinding.Custom");
	InputBindingCustomZoom              = GTM.AddNativeGameplayTag("InputBinding.Custom.Zoom");
	InputBindingCustomSelect            = GTM.AddNativeGameplayTag("InputBinding.Custom.Select");
	InputBindingCustomAllOrbitsShowHide = GTM.AddNativeGameplayTag("InputBinding.Custom.AllOrbits.ShowHide");
	InputBindingCustomMyOrbitShowHide   = GTM.AddNativeGameplayTag("InputBinding.Custom.MyOrbit.ShowHide");
	InputBindingCustomIngameMenuToggle  = GTM.AddNativeGameplayTag("InputBinding.Custom.IngameMenu.Toggle");
}

const FMyGameplayTags FMyGameplayTags::Singleton;

IMPLEMENT_MODULE(GameplayTagsModule, MyGameplayTags)
