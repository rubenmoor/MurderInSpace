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