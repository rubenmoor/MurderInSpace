#pragma once

#include "CoreMinimal.h"
#include "IHasMesh.h"
#include "MyPawn.h"
#include "Components/CapsuleComponent.h"
#include "MyComponents/MyCollisionComponent.h"

#include "MyPawn_SkeletalMesh.generated.h"

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API AMyPawn_SkeletalMesh : public AMyPawn, public IHasMesh, public IHasCollision
{
	GENERATED_BODY()
	
public:
	AMyPawn_SkeletalMesh();

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

	// event handlers
	virtual void OnConstruction(const FTransform& Transform) override;
};
