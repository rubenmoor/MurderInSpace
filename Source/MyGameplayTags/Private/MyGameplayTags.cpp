#include "MyGameplayTags.h"

#include "GameplayTagsManager.h"

DEFINE_LOG_CATEGORY(LogMyGameplayTags);

FMyGameplayTags::FMyGameplayTags(UGameplayTagsManager& GTM)
{
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

void FMyGameplayTags::AddTags(UGameplayTagsManager& GTM)
{
	check(Singleton == nullptr)
	Singleton = new FMyGameplayTags(GTM);
}


void GameplayTagsModule::StartupModule()
{
	IModuleInterface::StartupModule();
	auto& GTM = UGameplayTagsManager::Get();
	//GTM.OnLastChanceToAddNativeTags().AddRaw(this, &GameplayTagsModule::RegisterNativeGameplayTags);
	FMyGameplayTags::AddTags(GTM);
}

void GameplayTagsModule::RegisterNativeGameplayTags()
{
	auto& GTM = UGameplayTagsManager::Get();
	FMyGameplayTags::AddTags(GTM);
}

const FMyGameplayTags* FMyGameplayTags::Singleton = nullptr;

IMPLEMENT_MODULE(GameplayTagsModule, MyGameplayTags)