// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blackhole.generated.h"

class UNiagaraComponent;

UCLASS()
class MURDERINSPACE_API ABlackhole : public AActor
{
	GENERATED_BODY()
	
public:	
	ABlackhole();

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
};
