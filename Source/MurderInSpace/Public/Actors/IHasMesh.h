#pragma once

#include "IHasMesh.generated.h"

UINTERFACE(meta=(CannotImplementInterfaceInBlueprint))
class UHasMesh : public UInterface
{
    GENERATED_BODY()
};

class IHasMesh
{
    GENERATED_BODY()
    
public:
    // * actors that have one mesh component as root component return an empty array
    // * actors that have one or more mesh components, typically all children of the root component,
    //   return them in the array
    // * actors that don't have mesh components don't implement this interface
    virtual TArray<UPrimitiveComponent*> GetMeshComponents() const = 0;

    virtual FBoxSphereBounds GetBounds() const = 0;
    
    virtual double GetMyMass() const = 0;
    virtual FVector GetMyInertiaTensor() const { return FVector(1., 1., 1.); }
};

UINTERFACE(meta=(CannotImplementInterfaceBlueprint))
class UHasRandom : public UInterface
{
    GENERATED_BODY()
};

class IHasRandom
{
    GENERATED_BODY()

public:
    virtual int32 GetSeed() { return 0; }
};
