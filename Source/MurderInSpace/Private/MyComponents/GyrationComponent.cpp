#include "MyComponents/GyrationComponent.h"

#include "Actors/IHasMesh.h"
#include "Modes/MyGameState.h"
#include "Modes/MyState.h"
#include "Net/UnrealNetwork.h"

UGyrationComponent::UGyrationComponent()
{
    SetIsReplicatedByDefault(true);
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UGyrationComponent::FreezeState()
{
    if(!GetOwner<IHasMesh>())
    {
        UE_LOG(LogTemp, Error, TEXT("GetOwner<IHasMesh>() invalid: %s"), *GetOwner()->GetFullName())
    }
    else
    {
        RP_GyrationState = { GetOwner()->GetActorRotation() };
    }
}

void UGyrationComponent::BeginPlay()
{
    Super::BeginPlay();

    const auto* GS = GetWorld()->GetGameState<AMyGameState>();
    SetComponentTickEnabled(GS->bEnableGyration);
}

void UGyrationComponent::GyrationSetup()
{
    // TODO: meshes connected with sockets

    VecInertia = GetOwner<IHasMesh>()->GetMyInertiaTensor();

    if(GetOwnerRole() == ROLE_Authority)
    {
        checkf(GetOwner()->Implements<UHasGyration>(), TEXT("%s: missing interface: IHasGyration"), *GetFullName())
        // TODO: curve look-up
        const FVector Omega = GetOwner<IHasGyration>()->GetInitialOmega();
        RP_VecL = Omega * VecInertia.Length();
        L = RP_VecL.Length();
        E = Omega.Dot(RP_VecL) / 2.;
    }
    bGyrationSetupDone = true;
}

void UGyrationComponent::OnRep_GyrationState()
{
    GetOwner()->SetActorRotation(RP_GyrationState.Rot);
}

// Called every frame
void UGyrationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if(RP_VecL.IsZero())
    {
        return;
    }
    const FMatrix MatROld = FRotationMatrix(GetOwner()->GetActorRotation());
    const FMatrix MatInertiaReverse = FMatrix
            ( FVector(1./ VecInertia.X, 0, 0)
            , FVector(0, 1./ VecInertia.Y, 0)
            , FVector(0, 0, 1./ VecInertia.Z)
            , FVector(0, 0, 0)
            );
    VecOmega = (MatROld * MatInertiaReverse * MatROld.GetTransposed()).TransformFVector4(RP_VecL);
    const float Theta = VecOmega.Length();
    const FMatrix BNorm = FMatrix
            ( FVector(0., -VecOmega.Z, VecOmega.Y)
            , FVector(VecOmega.Z, 0., -VecOmega.X)
            , FVector(-VecOmega.Y, VecOmega.X, 0.)
            , FVector(0, 0, 0)
            ) * (1. / Theta);
    const FMatrix MatExp = FMatrix::Identity + BNorm * sin(Theta * DeltaTime) + BNorm * BNorm * (1. - cos(Theta * DeltaTime));
    const FMatrix MatRNew = MatExp * MatROld;

    GetOwner()->SetActorRotation(MatRNew.ToQuat());
    L = RP_VecL.Length();
    E = VecOmega.Dot(RP_VecL) / 2.;
}

#if WITH_EDITOR
void UGyrationComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
    Super::PostEditChangeChainProperty(PropertyChangedEvent);
    
    const FName Name = PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName();

    static const FName FNameL = GET_MEMBER_NAME_CHECKED(UGyrationComponent, L);
    static const FName FNameE = GET_MEMBER_NAME_CHECKED(UGyrationComponent, E);

    if(Name == FNameL)
    {
        RP_VecL *= L / RP_VecL.Length();
    }
    else if(Name == FNameE)
    {
        RP_VecL *= sqrt(2 * E * VecInertia.Length()) / L;
    }
}
#endif

void UGyrationComponent::OnComponentCreated()
{
    Super::OnComponentCreated();
    
    if(!HasAnyFlags(RF_Transient))
    {
        GyrationSetup();
    }
}

void UGyrationComponent::PostLoad()
{
    Super::PostLoad();
    if(!bGyrationSetupDone)
    {
        GyrationSetup();
    }
}

void UGyrationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UGyrationComponent, RP_VecL)
    DOREPLIFETIME(UGyrationComponent, RP_GyrationState)
}
