#pragma once

#include "CoreMinimal.h"
#include "MyGameplayAbility.h"
#include "GA_Embrace.generated.h"

UINTERFACE()
class UCanBeEmbraced : public UInterface
{
    GENERATED_BODY()
};

class ICanBeEmbraced
{
    GENERATED_BODY()

public:
    virtual float GetRadius() = 0;
};

UINTERFACE()
class UCanEmbrace : public UInterface
{
    GENERATED_BODY()
};

class ICanEmbrace
{
    GENERATED_BODY()

public:
    virtual FComponentBeginOverlapSignature& GetOnOverlapEmbraceSphere() = 0;
};

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API UGA_Embrace : public UMyGameplayAbility
{
    GENERATED_BODY()

public:
    UGA_Embrace();

protected:
    virtual FAbilityCoroutine ExecuteAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

    bool bStartEmbracing = false;
    float Span;

private:
    UFUNCTION()
    void MaybeStartEmbracing(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
