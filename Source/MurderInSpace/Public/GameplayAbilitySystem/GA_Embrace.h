#pragma once

#include "CoreMinimal.h"
#include "MyGameplayAbility.h"
#include "GA_Embrace.generated.h"

class UGE_TorqueCW;
class UGE_TorqueCCW;
class USphereComponent;

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
    virtual USphereComponent* GetEmbraceSphere() = 0;
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
    UPROPERTY(EditDefaultsOnly)
    double SmallPushAmount = 100.;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGE_TorqueCCW> GE_TorqueCCW;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGE_TorqueCW> GE_TorqueCW;
	
    virtual FAbilityCoroutine ExecuteAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

    bool bStartEmbracing = false;
    float Span;
    FVector VecN;

    UPROPERTY()
    AActor* OtherActor = nullptr;

private:
    UFUNCTION()
    void MaybeStartEmbracing(UPrimitiveComponent* _OverlappedComponent, AActor* InOtherActor, UPrimitiveComponent* _OtherComp, int32 _OtherBodyIndex, bool _bFromSweep, const FHitResult& _SweepResult);
};
