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

	virtual UPrimitiveComponent* GetMesh() const override final { return SkeletalMesh; }
	virtual float GetMyMass() const override { return pow(SkeletalMesh->Bounds.SphereRadius, 3); }
	
protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USkeletalMeshComponent> SkeletalMesh;
};
