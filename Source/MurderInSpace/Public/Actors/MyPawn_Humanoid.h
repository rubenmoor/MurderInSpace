#pragma once

#include "CoreMinimal.h"
#include "IHasMesh.h"
#include "MyPawn.h"
#include "Components/CapsuleComponent.h"
#include "MyComponents/MyCollisionComponent.h"

#include "MyPawn_Humanoid.generated.h"

UENUM(BlueprintType)
enum class EActionState : uint8
{
      Idle
    , Embracing
    , RotatingCW
    , RotatingCCW
    , KickPositioning
};

USTRUCT(BlueprintType)
struct FActionState
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
    EActionState State = EActionState::Idle;

    // determine which transition from `KickPositioning` to `Idle`: `true`: kick, `false` cancel kick
    UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
    bool bKickExecuteCancel;
};

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
    
    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
    FActionState RP_ActionState;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USkeletalMeshComponent> SkeletalMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UCapsuleComponent> CapsuleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UMyCollisionComponent> CollisionComponent;

    // event handlers

    void OnConstruction(const FTransform& Transform) override;
};
