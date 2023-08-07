#pragma once

#include "CoreMinimal.h"
#include "IHasMesh.h"
#include "MyPawn.h"
#include "Components/CapsuleComponent.h"
#include "MyComponents/MyCollisionComponent.h"

#include "MyPawn_Humanoid.generated.h"

class AHandThruster;

/**
 *  a pawn with a humanoid skeleton, like an astronaut
 */
UCLASS()
class MURDERINSPACE_API AMyPawn_Humanoid : public AMyPawn, public IHasMesh, public IHasCollision
{
    GENERATED_BODY()
    
public:
    AMyPawn_Humanoid();

    // IHasMesh
    virtual TArray<UPrimitiveComponent*> GetPrimitiveComponents() const override final { return {CapsuleComponent}; };
    virtual FBoxSphereBounds GetBounds() const override { return SkeletalMesh->Bounds; }

    // IHasCollision
    virtual UMyCollisionComponent* GetCollisionComponent() override { return CollisionComponent; }
    
protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USkeletalMeshComponent> SkeletalMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UCapsuleComponent> CapsuleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UMyCollisionComponent> CollisionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<AHandThruster> HandThrusterLeft;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<AHandThruster> HandThrusterRight;

    // event handlers

    void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;

    // handle gameplay cues

	// handle gameplay cues

	UFUNCTION()
	void GameplayCue_Accelerate_ShowThrusters(FGameplayTag Cue, EGameplayCueEvent::Type Event, const FGameplayCueParameters& Parameters);
    
	UFUNCTION()
    void GameplayCue_Accelerate_Fire(FGameplayTag Cue, EGameplayCueEvent::Type Event, const FGameplayCueParameters& Parameters);
};
