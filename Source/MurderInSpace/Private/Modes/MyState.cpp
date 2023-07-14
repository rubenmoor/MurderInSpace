#include "Modes/MyState.h"

#include "GameplayTagsManager.h"
#include "Modes/MyGameInstance.h"
#include "Modes/MyGameState.h"
#include "Modes/MyPlayerState.h"

DEFINE_LOG_CATEGORY(LogMyGame);

FPhysics UMyState::GetPhysics(const AMyGameState* GS)
{
    return GS->RP_Physics;
}

FPhysics UMyState::GetPhysicsAny(const UObject* Object)
{
    const auto* World = Object->GetWorld();
    const auto* GSGeneric = World->GetGameState<AGameState>();
    if(!IsValid(GSGeneric))
    {
        return PhysicsEditorDefault;
    }
    const AMyGameState* GS = World->GetGameState<AMyGameState>();
    if(!GS)
    {
        UE_LOG
            ( LogGameState
            , Error
            , TEXT("UMyState::GetPhysicsAny: %s: AMyGameState null; AGameState: %s")
            , *Object->GetName()
            , *GSGeneric->GetFullName()
            )
        return PhysicsEditorDefault;
    }
    return GetPhysics(GS);
}

FPlayerUI UMyState::GetPlayerUI(const AMyPlayerState* PS)
{
    return PS->PlayerUI;
}

FPlayerUI UMyState::GetPlayerUIAny(const UObject* Object, const FLocalPlayerContext& LPC)
{
    const APlayerState* PSGeneric = LPC.GetPlayerState<APlayerState>();
    if(!IsValid(PSGeneric))
    {
        return PlayerUIEditorDefault;
    }
    const AMyPlayerState* PS = LPC.GetPlayerState<AMyPlayerState>();
    if(!PS)
    {
        UE_LOG
            ( LogPlayerController
            , Error
            , TEXT("UStateLib::GetPlayerUIUnsafe: %s: AMyPlayerState null; APlayerState: %s")
            , *Object->GetFullName()
            , *PSGeneric->GetFullName()
            )
        return PlayerUIEditorDefault;
    }
    return GetPlayerUI(PS);
}

FInstanceUI UMyState::GetInstanceUI(const UMyGameInstance* GI)
{
    return GI->InstanceUI;
}

FInstanceUI UMyState::GetInstanceUIAny(const UObject* Object)
{
    const auto* World = Object->GetWorld();
    const auto* GIGeneric = World->GetGameInstance();
    if(!IsValid(GIGeneric))
    {
        return InstanceUIEditorDefault;
    }
    const UMyGameInstance* GI = World->GetGameInstance<UMyGameInstance>();
    if(!GI)
    {
        UE_LOG
        ( LogGameState
        , Error
        , TEXT("UStateLib::GetInstanceUIUnsafe: %s: UMyGameInstance null; UGameInstance: %s")
        , *Object->GetFullName()
        , *GIGeneric->GetFullName()
        )
        return InstanceUIEditorDefault;
    }
    return GetInstanceUI(GI);
}

void UMyState::WithPlayerUI(const UObject* Object, const FLocalPlayerContext& LPC, const std::function<void(FPlayerUI&)> Func)
{
    AMyPlayerState* PS = LPC.GetPlayerState<AMyPlayerState>();
    if(!PS)
    {
        const APlayerState* PSGeneric = LPC.GetPlayerState<APlayerState>();
        UE_LOG
            ( LogPlayerController
            , Error
            , TEXT("WithPlayerUI: %s: AMyPlayerState null; APlayerState: %s")
            , *Object->GetFullName()
            , PSGeneric ? *PSGeneric->GetFullName() : TEXT("null")
            )
        return;
    }
    Func(PS->PlayerUI);
}

void UMyState::WithInstanceUI(const UObject* Object, const std::function<void(FInstanceUI&)> Func)
{
    const UWorld* World = Object->GetWorld();
    UMyGameInstance* GI = World->GetGameInstance<UMyGameInstance>();
    if(!GI)
    {
        const UGameInstance* GIGeneric = World->GetGameInstance();
        UE_LOG
        ( LogGameState
        , Error
        , TEXT("UMyState::WithInstanceUI: %s: UMyGameInstance null; UGameInstance: %s")
        , *Object->GetFullName()
        , GIGeneric ? *GIGeneric->GetFullName() : TEXT("null")
        )
        return;
    }
    Func(GI->InstanceUI);
}

void UMyState::WithPhysics(const UObject* Object, const std::function<void(FPhysics&)> Func)
{
    const UWorld* World = Object->GetWorld();
    AMyGameState* GS = World->GetGameState<AMyGameState>();
    if(!GS)
    {
        const AGameState* GSGeneric = World->GetGameState<AGameState>();
        UE_LOG
        ( LogGameState
        , Error
        , TEXT("UMyState::WithPhysics: %s: AMyGameState null; AGameState: %s")
        , *Object->GetFullName()
        , GSGeneric ? *GSGeneric->GetFullName() : TEXT("null")
        )
        return;
    }
    Func(GS->RP_Physics);
}
