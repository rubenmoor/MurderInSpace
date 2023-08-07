#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Animation/AnimInstance.h"
#include "AstronautAnimInstance.generated.h"

UENUM(BlueprintType)
enum class ERotationDirection : uint8
{
	  Clockwise
	, Counterclockwise
};

class AMyPawn_Humanoid;
class AMyPawn;
/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UAstronautAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly)
	FGameplayTagBlueprintPropertyMap GameplayTagPropertyMap;

	UFUNCTION(BlueprintCallable)
	void HandleStateFullyBlended();

    // event handlers

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif

    virtual void NativeInitializeAnimation() override;
};
