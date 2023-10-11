#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "HandThruster.generated.h"

class UNiagaraComponent;

UCLASS()
class MURDERINSPACE_API AHandThruster : public AActor
{
    GENERATED_BODY()

public:
    AHandThruster();

    void EnableBurn(bool InBEnabled);

    void SetEnableOverlap(bool InBEnabled);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UNiagaraComponent> NiagaraComponent;
};
