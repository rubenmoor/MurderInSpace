#include "Modes/MyGameState.h"

#include "Actors/Blackhole.h"
#include "Kismet/GameplayStatics.h"
#include "Orbit/Orbit.h"
#include "Lib/FunctionLib.h"
#include "Net/UnrealNetwork.h"

ABlackhole* AMyGameState::GetBlackhole() const
{
    // not ideal, but if I don't use `UGameplayStatics` to get the black hole,
    // I have to spawn it at run-time and it wouldn't be there in the level in the editor
    return Cast<ABlackhole>(UGameplayStatics::GetActorOfClass(this, BlackholeClass));
}

void AMyGameState::OnRep_Physics()
{
    UWorld* World = GetWorld();
    auto Filter = [this, World] (const AOrbit* Orbit)
    {
        return Orbit->GetWorld() == World
            && Orbit->GetIsInitialized();
    };
    for(TMyObjectIterator<AOrbit> IOrbit(Filter); IOrbit; ++IOrbit)
    {
        (*IOrbit)->Update(RP_Physics);
    }
}

#if WITH_EDITOR
void AMyGameState::BeginPlay()
{
    Super::BeginPlay();
    check(IsValid(BlackholeClass))
}

void AMyGameState::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
    Super::PostEditChangeChainProperty(PropertyChangedEvent);
    
    const FName Name = PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName();

    // TODO: confirm that this is how it actually works
    static const FName FNamePhysics = GET_MEMBER_NAME_CHECKED(AMyGameState, RP_Physics);
    
    if(Name == FNamePhysics)
    {
        for(TMyObjectIterator<AOrbit> IOrbit; IOrbit; ++IOrbit)
        {
            auto* Orbit = *IOrbit;
            Orbit->UpdateByInitialParams(RP_Physics);
        }
    }
}
#endif

void AMyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AMyGameState, RP_Physics)
}