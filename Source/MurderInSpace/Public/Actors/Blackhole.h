#pragma once

#include "CoreMinimal.h"
#include "Blackhole.generated.h"

class UPointLightComponent;
class UNiagaraComponent;

UCLASS()
class MURDERINSPACE_API ABlackhole : public AActor
{
	GENERATED_BODY()
	
public:	
	ABlackhole();

	// return IsValid(Actor), i.e. is alive
	UFUNCTION()
	bool ApplyTideForceDamage(AActor* Actor, double RKepler);

	UFUNCTION(BlueprintPure)
	double GetKillRadius() { return KillRadius; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly);
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Required")
	TObjectPtr<UStaticMeshComponent> EventHorizon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Required")
	TObjectPtr<UStaticMeshComponent> GravitationalLens;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Required")
	TObjectPtr<UNiagaraComponent> NS_Vortex;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Required")
	TObjectPtr<UStaticMeshComponent> InnerDisc;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Required")
	TObjectPtr<UNiagaraComponent> NS_Jet;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Required")
	TObjectPtr<UNiagaraComponent> NS_Jetstream;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Required")
	TObjectPtr<UPointLightComponent> PointLight;

	// black hole damage
	
	// distance to black hole: any orbiting thing closer than the `KillRadius` immediately gets destroyed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double KillRadius = 100.;
	
	// tidal force damage parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double TFVScale = 1000.;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double TFHScale = 0.2;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double TFYOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double TideForceDamageRadius = 500.;

	// the damage value, missing a y-Offset
	double DamageCurve(double R) const;

	// radiation damage parameters
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double RadiationDamageRadius = 1500;
};
