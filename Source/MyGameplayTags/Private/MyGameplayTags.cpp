#include "MyGameplayTags.h"

#include "GameplayTagsManager.h"

FMyGameplayTags::FMyGameplayTags()
{
	auto& GTM = UGameplayTagsManager::Get();

	Acceleration = GTM.AddNativeGameplayTag("Acceleration");
	AccelerationTranslational = GTM.AddNativeGameplayTag("Acceleration.Translational");
	AccelerationRotational = GTM.AddNativeGameplayTag("Acceleration.Rotational");
	AccelerationRotationalDuration = GTM.AddNativeGameplayTag("Acceleration.Rotational.Duration");
	
	Ability = GTM.AddNativeGameplayTag("Ability");
	AbilityAccelerate = GTM.AddNativeGameplayTag("Ability.Accelerate");
	AbilityRotate = GTM.AddNativeGameplayTag("Ability.Rotate");
	AbilityMoveTowardsCircle = GTM.AddNativeGameplayTag("Ability.MoveTowardsCircle");

	// deprecated
	HasTorque    = GTM.AddNativeGameplayTag("Movement.HasTorque");
	HasTorqueCCW = GTM.AddNativeGameplayTag("Movement.HasTorque.CCW");
	HasTorqueCW  = GTM.AddNativeGameplayTag("Movement.HasTorque.CW");
}

const FGameplayTag& FMyGameplayTags::GetInputActionTag(EInputAction InputAction) const
{
    return InputActionTags[static_cast<uint8>(InputAction)];
}

FMyGameplayTags FMyGameplayTags::MyGameplayTags;

IMPLEMENT_MODULE(GameplayTagsModule, MyGameplayTags)