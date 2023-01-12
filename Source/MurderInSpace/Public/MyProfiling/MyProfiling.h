#pragma once

#include "CoreMinimal.h"

/**
 * https://discord.com/channels/187217643009212416/1014245197623869491/1014383504898920459
 * Custom scope cycle object for accurate execution cost calculation.
 * Essentially same as SCOPE_CYCLE_COUNTER, except does not require any
 * stat category and logs the result instead of using stat system.
 */
class FCustomScopeCycle
{

public:
    FCustomScopeCycle() = delete;

    FCustomScopeCycle(const FString& InContext)
    {
#if !UE_BUILD_SHIPPING
        Context = InContext;
        End = 0;
        Start = FPlatformTime::Cycles64();
#endif
    }
    
    ~FCustomScopeCycle()
    {
#if !UE_BUILD_SHIPPING
        End = FPlatformTime::Cycles64();
        UE_LOG(LogTemp, Warning, TEXT("Executing %s took %f milliseconds "), *Context, static_cast<float>(FPlatformTime::ToMilliseconds64(End - Start)));
#endif
    }

private:
    uint64 Start;
    uint64 End;

    FString Context;
};