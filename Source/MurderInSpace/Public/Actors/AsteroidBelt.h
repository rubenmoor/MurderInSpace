#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Actors/DynamicAsteroid.h"
#include "Components/SphereComponent.h"

#include "AsteroidBelt.generated.h"

UCLASS()
class MURDERINSPACE_API AAsteroidBelt : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AAsteroidBelt();

protected:
	UPROPERTY(EditAnywhere, Category="Generation")
	TSubclassOf<ADynamicAsteroid> DynamicAsteroidClass;
	
    UPROPERTY(EditAnywhere, Category="Generation")
    TObjectPtr<UCurveFloat> CurveAsteroidSize;

    UPROPERTY(EditAnywhere, Category="Generation")
    TObjectPtr<UCurveFloat> CurveAsteroidDistance;

    UPROPERTY(EditInstanceOnly, Category="Generation")
    int32 NumAsteroids = 0;

    UPROPERTY(EditAnywhere, Category="Generation")
    double MinAsteroidSize = 45.;

    UPROPERTY(EditAnywhere, Category="Generation")
    double MaxAsteroidSize = 2000.;

    UPROPERTY(EditAnywhere, Category="Generation")
    double Width = 1000.;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USceneComponent> Root;
    
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
    float MakeAsteroidSize(const FRandomStream& RandomStream) const;

    UFUNCTION(BlueprintPure)
    FVector MakeAsteroidDistance(FPhysics Physics, const FRandomStream& RandomStream) const;

    // Position is a value between 0 and 1
    UFUNCTION(BlueprintPure)
    float MakeAsteroidAlpha(const FRandomStream& RandomStream);
};
