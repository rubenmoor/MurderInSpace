#pragma once

#include "CoreMinimal.h"
#include "IHasMesh.h"
#include "MyPawn.h"

#include "MyPawn_SkeletalMesh.generated.h"

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API AMyPawn_SkeletalMesh : public AMyPawn, public IHasMesh
{
	GENERATED_BODY()
	
public:
	AMyPawn_SkeletalMesh();

	virtual TArray<UPrimitiveComponent*> GetMeshComponents() const override final { return {SkeletalMesh}; };
	virtual FBoxSphereBounds GetBounds() const override { return GetRootComponent()->Bounds; }
	virtual double GetMyMass() const override { return pow(SkeletalMesh->Bounds.SphereRadius, 3); }
	
protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USkeletalMeshComponent> SkeletalMesh;
};
