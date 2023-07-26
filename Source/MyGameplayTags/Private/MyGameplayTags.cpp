#include "MyGameplayTags.h"

#include "GameplayTagsManager.h"

FMyGameplayTags::FMyGameplayTags()
{
	auto& GTM = UGameplayTagsManager::Get();

	Acceleration                   = GTM.AddNativeGameplayTag("Acceleration");
	AccelerationTranslational      = GTM.AddNativeGameplayTag("Acceleration.Translational");
	AccelerationRotational         = GTM.AddNativeGameplayTag("Acceleration.Rotational");
	AccelerationRotationalDuration = GTM.AddNativeGameplayTag("Acceleration.Rotational.Duration");
	AccelerationRotationalCCW      = GTM.AddNativeGameplayTag("Acceleration.Rotational.CCW");
	AccelerationRotationalCW       = GTM.AddNativeGameplayTag("Acceleration.Rotational.CW");
	
	Ability                  = GTM.AddNativeGameplayTag("Ability");
	AbilityAccelerate        = GTM.AddNativeGameplayTag("Ability.Accelerate");
	AbilityLookAt            = GTM.AddNativeGameplayTag("Ability.LookAt");
	AbilityMoveTowardsCircle = GTM.AddNativeGameplayTag("Ability.MoveTowardsCircle");
	AbilityEmbrace           = GTM.AddNativeGameplayTag("Ability.Embrace");
	AbilityKick              = GTM.AddNativeGameplayTag("Ability.Kick");

	GiveInitiallyToMyPawn    = GTM.AddNativeGameplayTag("GiveTo.Initial.MyPawn");

	GameplayCue              = GTM.AddNativeGameplayTag("GameplayCue");
	
	InputBindingCustom                  = GTM.AddNativeGameplayTag("InputBinding.Custom");
	InputBindingCustomZoom              = GTM.AddNativeGameplayTag("InputBinding.Custom.Zoom");
	InputBindingCustomSelect            = GTM.AddNativeGameplayTag("InputBinding.Custom.Select");
	InputBindingCustomAllOrbitsShowHide = GTM.AddNativeGameplayTag("InputBinding.Custom.AllOrbits.ShowHide");
	InputBindingCustomMyOrbitShowHide   = GTM.AddNativeGameplayTag("InputBinding.Custom.MyOrbit.ShowHide");
	InputBindingCustomIngameMenuToggle  = GTM.AddNativeGameplayTag("InputBinding.Custom.IngameMenu.Toggle");
}

FMyGameplayTags FMyGameplayTags::MyGameplayTags;

IMPLEMENT_MODULE(GameplayTagsModule, MyGameplayTags)