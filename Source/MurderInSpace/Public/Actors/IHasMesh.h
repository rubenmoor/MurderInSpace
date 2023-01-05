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
};
