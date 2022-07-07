// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Orbit.h"
#include "Components/ActorComponent.h"
#include "OrbitDataComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MURDERINSPACE_API UOrbitDataComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UOrbitDataComponent();

	// public members

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AOrbit> Orbit;

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
	void SpawnOrbit(UClass* OrbitClass);

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

	// event handlers

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
	#endif
	
};
