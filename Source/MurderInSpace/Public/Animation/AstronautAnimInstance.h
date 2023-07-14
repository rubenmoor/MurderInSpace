#pragma once

#include "CoreMinimal.h"
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
    // if the animation blueprint needs access to Editor Assets of e.g. BP_SomePawn,
    // I have to add a `TSubclassOf<AMyPawn>` and so on; I don't think that's the case
    UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
    TObjectPtr<AMyPawn_Humanoid> MyPawn;

    // event handlers

    virtual void NativeInitializeAnimation() override;
};
