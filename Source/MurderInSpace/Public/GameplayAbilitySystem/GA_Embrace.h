#pragma once

#include "CoreMinimal.h"
#include "MyGameplayAbility.h"
#include "GA_Embrace.generated.h"

class UGE_RotateCCW;
class UGE_RotateCW;
class UGE_MoveBackward;
class UGE_MoveForward;
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
    virtual AActor* GetEmbracingActor() = 0;
    virtual void SetEmbracingActor(AActor* InActor) = 0;
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
    virtual AActor* GetEmbracedActor() = 0;
    virtual void SetEmbracedActor(AActor* InActor) = 0;
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

    UPROPERTY(EditDefaultsOnly)
    double EmbraceDistanceModifier = 50.;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGE_RotateCCW> GE_RotateCCW;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGE_RotateCW> GE_RotateCW;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGE_MoveForward> GE_MoveForward;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGE_MoveBackward> GE_MoveBackward;
	
    virtual FAbilityCoroutine ExecuteAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

    bool bStartEmbracing = false;
    float Span;
    FVector VecN;

    UPROPERTY()
    AActor* OtherActor = nullptr;

private:
    UFUNCTION()
    void MaybeStartEmbracing(UPrimitiveComponent* _OverlappedComponent, AActor* InOtherActor, UPrimitiveComponent* _OtherComp, int32 _OtherBodyIndex, bool _bFromSweep, const FHitResult& _SweepResult);

    double CalcAngle(const AActor* Actor1, const AActor* Actor2);
    double CalcEmbraceDistance(const AActor* Actor1, const AActor* Actor2);

    UPROPERTY()
    bool bRotating = false;
    
    UPROPERTY()
    bool bMovingForward = false;
};
