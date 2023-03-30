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
    virtual UPrimitiveComponent* GetMesh() const = 0;
    virtual float GetMyMass() const = 0;
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
