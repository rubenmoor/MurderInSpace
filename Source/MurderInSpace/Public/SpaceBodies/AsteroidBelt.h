#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Spacebodies/DynamicAsteroid.h"
#include "Components/SphereComponent.h"
#include "Modes/MyGameState.h"

#include "AsteroidBelt.generated.h"

class UNiagaraComponent;

USTRUCT(BlueprintType)
struct FAsteroidGroup
{
    GENERATED_BODY()
    
	UPROPERTY(EditAnywhere, Category="Generation")
	TSubclassOf<ADynamicAsteroid> DynamicAsteroidClass;

    UPROPERTY(EditAnywhere, Category="Generation")
    float RelativeFrequency = 1.0;
    
    UPROPERTY(EditAnywhere, Category="Generation")
    double MinAsteroidSize = 45.;

    UPROPERTY(EditAnywhere, Category="Generation")
    double MaxAsteroidSize = 2000.;

};

UCLASS()
class MURDERINSPACE_API AAsteroidBelt : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AAsteroidBelt();

    void ClearAsteroidPointer(ADynamicAsteroid* Asteroid) { Asteroids.RemoveSingle(Asteroid); }
    
protected:
    // if disabled, the asteroid belt disappears with all its asteroids
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Generation")
    bool bEnabled = true;
    
    UPROPERTY(VisibleAnywhere, Category="Generation")
    TArray<ADynamicAsteroid*> Asteroids;
    
    UPROPERTY(EditAnywhere, Category="Generation")
    TArray<FAsteroidGroup> AsteroidGroups;
    
    UPROPERTY(EditAnywhere, Category="Generation")
    TObjectPtr<UCurveFloat> CurveAsteroidSize;

    UPROPERTY(EditAnywhere, Category="Generation")
    TObjectPtr<UCurveFloat> CurveAsteroidDistance;

    UPROPERTY(EditInstanceOnly, Category="Generation")
    int32 NumAsteroids = 0;

    UPROPERTY(EditAnywhere, Category="Generation")
    double Width = 1000.;

    UPROPERTY(EditAnywhere, Category="Generation")
    float MeshResolution = 1.;

    UPROPERTY(EditAnywhere, Category="Generation")
    bool bRecomputeNormals = false;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USceneComponent> Root;

    // fog
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Fog")
    TObjectPtr<UNiagaraComponent> NS_Fog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fog")
    float ParticleSizeMax = 1000.;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fog")
    float ParticleSizeMin = 200.;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fog")
    double FogDensity = 0.01;
    
    // create boundaries that correspond the extend of the asteroid belt
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Sphere;

    // event handlers
    virtual void Destroyed() override;
#if WITH_EDITOR
    virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
    virtual void OnConstruction(const FTransform& Transform) override;
#endif

private:
    UFUNCTION(BlueprintCallable, CallInEditor, Category="Generation")
    void BuildAsteroids();
    
    UFUNCTION(BlueprintPure)
    float MakeAsteroidSize(const FAsteroidGroup& AsteroidGroup) const;

    UFUNCTION(BlueprintPure)
    FVector MakeAsteroidDistance(FPhysics Physics) const;

    UFUNCTION(BlueprintCallable)
    FAsteroidGroup PickAsteroidGroup();

    FRandomStream RandomStream;
};
