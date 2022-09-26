// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GyrationComponent.generated.h"

/*
 * physical rotation of objects in space
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MURDERINSPACE_API UGyrationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGyrationComponent();

	UFUNCTION(BlueprintCallable)
	void SetBody(UPrimitiveComponent* InBody) { Body = InBody; }
	
protected:
	
	// event handlers
	
	virtual void BeginPlay() override;

	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
	
	// private members

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UPrimitiveComponent> Body;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	FVector VecInertia = FVector::Zero();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	FVector VecL = FVector::Zero();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	float L = 0.;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	float E = 0.;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kepler")
	FVector VecOmega = FVector::Zero();
	
	// private methods
	
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
