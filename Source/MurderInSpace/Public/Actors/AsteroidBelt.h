#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Actors/DynamicAsteroid.h"

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
    int32 NumAsteroids = 10;

    UPROPERTY(EditAnywhere, Category="Generation")
    float MinAsteroidSize = 1.;

    UPROPERTY(EditAnywhere, Category="Generation")
    int32 FractalNumber = 3;

    // event handlers
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void Destroyed() override;
#if WITH_EDITOR
    virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

private:
    UFUNCTION(BlueprintCallable)
    void BuildAsteroids();
    
    UFUNCTION(BlueprintPure)
    static float FractalNoise(int32 N, float Seed);
    
    UFUNCTION(BlueprintPure)
    float MakeAsteroidSize(float Seed) const;

};
