#pragma once

#include "CoreMinimal.h"
#include "MyPawn_Humanoid.h"
#include "Components/SceneCaptureComponent2D.h"

#include "MyCharacter.generated.h"

class AHandThruster;
class UCameraComponent;
class USpringArmComponent;
class UNiagaraComponent;

/**
 * The inheritance hierarchy goes like
 *    APawn
 * -> AMyPawn
 * -> AMyPawn_SkeletalMesh
 * -> AMyCharacter
 *
 * Thus not using `ACharacter`
 * 
 */
UCLASS()
class MURDERINSPACE_API AMyCharacter final : public AMyPawn_Humanoid
{
    GENERATED_BODY()

    AMyCharacter();

public:

    UFUNCTION(BlueprintCallable)
    void UpdateSpringArm(uint8 InCameraPosition);

    // make stuff visible for the possessing player
    UFUNCTION(BlueprintCallable)
    void ShowEffects();
    
    // hide the astronaut when the camera gets to First-Person view
    UFUNCTION(BlueprintCallable)
    void SetVisibility(bool bVisibility);

    UFUNCTION(BlueprintPure)
    float GetSpringArmLength() const;

    UFUNCTION(BlueprintPure)
    FLinearColor GetTempSplineMeshColor() const { return TempSplineMeshColor; }

    UFUNCTION(BlueprintCallable)
    void AddComponentToSceneCapture(UPrimitiveComponent* Component) const
    {
        SceneCapture->ShowOnlyComponent(Component);
    }

    UFUNCTION(BlueprintCallable)
    UTextureRenderTarget2D* SetNewRenderTarget(int32 Width, int32 Height);

    UFUNCTION(BlueprintCallable)
    void ClearRenderTarget()
    {
        SceneCapture->TextureTarget = nullptr;
    }
    
    // IHasMesh
    //virtual UPrimitiveComponent* GetMesh() override { return SkeletalMesh; }

protected:

    // event handlers
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;
    

#if WITH_EDITOR
    virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;

protected:
#endif
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor TempSplineMeshColor;
    
    // components

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UNiagaraComponent> Dust;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USpringArmComponent> SpringArm;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UCameraComponent> Camera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USceneCaptureComponent2D> SceneCapture;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UStaticMeshComponent> Visor;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UStaticMeshComponent> VisorFrame;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USceneComponent> StarAnchor;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UNiagaraComponent> StarsClose;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UNiagaraComponent> StarsDistant;

    // camera settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Configuration")
    float CameraLengthExponent = 2.;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Configuration")
    float CameraLengthConst = 500.;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Configuration")
    float CameraLengthFactor = 100.;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Configuration")
    float CameraRotationConst = 30.;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Configuration")
    float CameraRotationFactor = 7.;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Configuration")
    uint8 CameraPosition = 2;
};
