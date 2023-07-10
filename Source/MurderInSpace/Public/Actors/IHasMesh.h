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
    // return relevant primitive components for collision
    // can be static meshes, capsule component, or even skeletal mesh
    virtual TArray<UPrimitiveComponent*> GetPrimitiveComponents() const = 0;

    // for mouse interaction/selection of orbiting bodies
    virtual FBoxSphereBounds GetBounds() const = 0;
    
    virtual double GetMyMass() const = 0;
    virtual FVector GetMyInertiaTensor() const { return FVector(1., 1., 1.); }
};
