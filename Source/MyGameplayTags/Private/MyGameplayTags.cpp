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
	AbilityRotate            = GTM.AddNativeGameplayTag("Ability.Rotate");
	AbilityMoveTowardsCircle = GTM.AddNativeGameplayTag("Ability.MoveTowardsCircle");

	GiveInitiallyToMyPawn    = GTM.AddNativeGameplayTag("GiveTo.Initial.MyPawn");
}

FMyGameplayTags FMyGameplayTags::MyGameplayTags;

IMPLEMENT_MODULE(GameplayTagsModule, MyGameplayTags)