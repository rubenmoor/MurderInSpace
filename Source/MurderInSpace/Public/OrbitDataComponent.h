// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Orbit.h"
#include "Components/ActorComponent.h"
#include "OrbitDataComponent.generated.h"

/*
 *  The orbit data component is limited to containing data relevant
 *  to orbital motion. It links to the orbit actor via property.
 *
 *  The orbit data component ticks: It gets the new actor location by
 *  passing the orbit data to `AdvanceOnSpline` of the orbit actor.
 *
 *  Having the orbit data component tick instead of the actor reduces boilerplate
 *  in the actor. Having the orbit data component tick instead of the orbit
 *  actor avoids the situation of having a stationary actor moving another actor.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MURDERINSPACE_API UOrbitDataComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UOrbitDataComponent();

	// public members

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AOrbit> Orbit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMaterialInstance> SplineMeshMaterial;

	// member functions

	UFUNCTION(BlueprintCallable)
	float GetCircleVelocity(float Alpha, FVector VecF1) const;

	UFUNCTION(BlueprintCallable)
	FVector GetVecR() const { return GetOwner()->GetActorLocation(); };

	UFUNCTION(BlueprintCallable)
	void SetVelocity(FVector _VecVelocity, float Alpha, FVector VecF1);

	UFUNCTION(BlueprintCallable)
	void AddVelocity(FVector _VecVelocity, float Alpha, FVector VecF1);
	
	UFUNCTION(BlueprintCallable)
	void SpawnOrbit(UClass* OrbitClass, UMaterialInstance* Material);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FVector GetVecVelocity() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetVelocity() const;
	
protected:
	// private members

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector VecVelocity;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Velocity;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VelocityVCircle;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bInitialized = false;

	// this is a pointer to the physical body that orbits and has
	// simulated physics; it can be anywhere in the scene tree of the
	// owning actor
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UPrimitiveComponent> Body;
	
	// event handlers
	
public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
	#endif
	
};
