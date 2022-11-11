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

	virtual UPrimitiveComponent* GetMesh() override final { return SkeletalMesh; }
	
protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USkeletalMeshComponent> SkeletalMesh;
    
};
