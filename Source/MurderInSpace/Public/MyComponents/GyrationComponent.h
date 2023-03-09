// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GyrationComponent.generated.h"

/*
 * rotational state of an object for network replication
 */
USTRUCT(BlueprintType)
struct FGyrationState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FRotator Rot = FRotator::ZeroRotator;
};

/*
 * physical rotation of objects in space
 *
 * requires the owning actor to implement `IHasMesh`
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MURDERINSPACE_API UGyrationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGyrationComponent();

	UFUNCTION(BlueprintCallable)
	void FreezeState();

protected:
	
	// event handlers
	
	virtual void BeginPlay() override;

	#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
	virtual void OnComponentCreated() override;
	virtual void PostLoad() override;
	#endif
	
	// private members

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gyration")
	FVector VecInertia = FVector::Zero();
	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category="Gyration")
	FVector RP_VecL = FVector::Zero();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gyration")
	float L = 0.;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gyration")
	float E = 0.;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gyration")
	FVector VecOmega = FVector::Zero();

	// replication
	UPROPERTY(ReplicatedUsing=OnRep_GyrationState, VisibleAnywhere, BlueprintReadOnly)
	FGyrationState RP_GyrationState;

	FRandomStream RandomStream;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bGyrationSetupDone = false;
	
	// private methods
	UFUNCTION()
	void GyrationSetup();
	
	UFUNCTION()
	void OnRep_GyrationState();
	
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
