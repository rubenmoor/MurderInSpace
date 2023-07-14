#include "Animation/AstronautAnimInstance.h"

#include "Actors/MyPawn_Humanoid.h"

void UAstronautAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    MyPawn = Cast<AMyPawn_Humanoid>(TryGetPawnOwner());
}
