#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MyDestructibleComponent.generated.h"

UINTERFACE()
class UHasDestructible : public UInterface
{
	GENERATED_BODY()
};

class IHasDestructible
{
	GENERATED_BODY()

public:
	
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MURDERINSPACE_API UMyDestructibleComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMyDestructibleComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
