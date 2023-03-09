// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyState.h"
#include "GameFramework/GameState.h"
#include "MyGameState.generated.h"

/**
 * 
 */
UCLASS()
class MURDERINSPACE_API AMyGameState : public AGameState
{
	GENERATED_BODY()

	friend class UMyState;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableGyration = true;

	static AMyGameState* Get(const UWorld* World) { return World->GetGameState<AMyGameState>(); }
protected:

	// game world parameters to be edited in blueprint and to be used in game
	UPROPERTY(ReplicatedUsing=OnRep_Physics, EditAnywhere, BlueprintReadWrite)
	FPhysics RP_Physics = PhysicsEditorDefault;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double GyrationOmegaInitial = 0.1;

	// event handlers

	virtual void PostInitializeComponents() override;

	#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
	#endif

	// replication
	
	UFUNCTION()
	void OnRep_Physics();
};
