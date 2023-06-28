#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyDebris.generated.h"

UCLASS()
class MURDERINSPACE_API AMyDebris : public AActor
{
	GENERATED_BODY()
	
public:	
	AMyDebris();

	void SetRadius(double InRadius) { Radius = InRadius; }
	void SetLifetime(double InLifetime) { Lifetime = InLifetime; }
	void SetCoMVelocity(FVector InVecVelocity) { VecCoMVelocity = InVecVelocity; }
	void SetNumParticles(int32 InNum) { Num = InNum; }
	void SetAverageScale(double InScale) { AverageScale = InScale; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UInstancedStaticMeshComponent> ISMComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMesh> Mesh;

	double Radius;

	// life-time in seconds; when age reaches lifetime, the particle dies
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	double Lifetime = 3.;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	double TraversalAttenuation = 0.1;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	double InitialAttenuation = 0.3;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	double ScaleFactor = 1.;
	
	double AverageScale;
	double Age = 0.;
	int32 Num;

	FVector VecCoMVelocity;
	TArray<FVector> IndividualVelocities;
	TArray<double> Scales;

	FVector ToTangential(FVector InVec, FVector InVecRKepler);
};
