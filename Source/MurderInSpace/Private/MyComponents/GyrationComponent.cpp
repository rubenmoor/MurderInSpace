// Fill out your copyright notice in the Description page of Project Settings.


#include "MyComponents/GyrationComponent.h"

#include "Actors/IHasMesh.h"
#include "Modes/MyGameState.h"
#include "Modes/MyState.h"
#include "Net/UnrealNetwork.h"
#include "Modes/MyGameInstance.h"

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
        UE_LOG(LogTemp, Error, TEXT("GetOwner<IHasMesh>(): %s"), *GetOwner()->GetFullName())
    }
    else
    RP_GyrationState = { GetOwner<IHasMesh>()->GetMesh()->GetComponentRotation() };
}

void UGyrationComponent::BeginPlay()
{
    Super::BeginPlay();

    const auto* GS = GetWorld()->GetGameState<AMyGameState>();
    SetComponentTickEnabled(GS->bEnableGyration);

}

void UGyrationComponent::GyrationSetup()
{
    UE_LOG(LogMyGame, Warning, TEXT("%s: GyrationSetup: GetOwner() %s")
        , *GetFullName()
        , *GetOwner()->GetFName().ToString()
        )
        
    RandomStream.Initialize(GetOwner()->GetFName());
    // TODO: meshes connected with sockets
    VecInertia = GetOwner<IHasMesh>()->GetMesh()->GetInertiaTensor();
    if(VecInertia.IsNearlyZero())
    {
        UE_LOG(LogMyGame, Error, TEXT("%s: VecIntertia nearly zero: (%f, %f, %f), setting to (1, 1, 1)")
            , *GetFullName()
            , VecInertia.X
            , VecInertia.Y
            , VecInertia.Z
            )
        VecInertia = FVector(1., 1., 1.);
    }

    if(GetOwnerRole() == ROLE_Authority)
    {
        if(RP_VecL.IsZero())
        {
            // TODO: curve look-up
            const float OmegaInitial = RandomStream.FRand();
            const FVector VRand = RandomStream.VRand();
            RP_VecL = VRand * OmegaInitial * VecInertia.Length();
            UE_LOG
                ( LogMyGame
                , Display
                , TEXT("%s: Initializing angular momentum: (%f, %f, %f), OmegaInitial: %f, Vrand: (%f, %f, %f)")
                , *GetFullName()
                , RP_VecL.X
                , RP_VecL.Y
                , RP_VecL.Z
                , OmegaInitial
                , VRand.X
                , VRand.Y
                , VRand.Z
                )
        }
        else
        {
            UE_LOG
                ( LogMyGame
                , Error
                , TEXT("%s: Angular momentum set already: (%f, %f, %f), Owner role: %d")
                , *GetFullName()
                , RP_VecL.X
                , RP_VecL.Y
                , RP_VecL.Z
                , GetOwnerRole()
                )
        }
    }
    bGyrationSetupDone = true;
}

void UGyrationComponent::OnRep_GyrationState()
{
    GetOwner<IHasMesh>()->GetMesh()->SetWorldRotation(RP_GyrationState.Rot);
}

// Called every frame
void UGyrationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UPrimitiveComponent* Body = GetOwner<IHasMesh>()->GetMesh();
    const FMatrix MatROld = FRotationMatrix(Body->GetComponentRotation());
    const FMatrix MatInertiaReverse = FMatrix
            ( FVector(1./ VecInertia.X, 0, 0)
            , FVector(0, 1./ VecInertia.Y, 0)
            , FVector(0, 0, 1./ VecInertia.Z)
            , FVector(0, 0, 0)
            );
    VecOmega = (MatROld * MatInertiaReverse * MatROld.GetTransposed()).TransformFVector4(RP_VecL);
    if(VecOmega.IsNearlyZero())
    {
        if(VecOmega.IsZero())
        {
            UE_LOG(LogMyGame, Error, TEXT("%s: VecOmega zero, skipping."), *GetFullName())
            return;
        }
        UE_LOG(LogMyGame, Warning, TEXT("%s: VecOmega nearly zero, just a warning."), *GetFullName())
    }
    const float Theta = VecOmega.Length();
    const FMatrix BNorm = FMatrix
            ( FVector(0., -VecOmega.Z, VecOmega.Y)
            , FVector(VecOmega.Z, 0., -VecOmega.X)
            , FVector(-VecOmega.Y, VecOmega.X, 0.)
            , FVector(0, 0, 0)
            ) * (1. / Theta);
    const FMatrix MatExp = FMatrix::Identity + BNorm * sin(Theta * DeltaTime) + BNorm * BNorm * (1. - cos(Theta * DeltaTime));
    const FMatrix MatRNew = MatExp * MatROld;

    //FHitResult HitResult;
    // sweeping isn't implemented for rotation
    // instead I deal with hits in AOrbit::Tick, where translation happens
    //Body->SetWorldRotation(MatRNew.ToQuat(), true, HitResult);
    Body->SetWorldRotation(MatRNew.ToQuat());
    L = RP_VecL.Length();
    E = VecOmega.Dot(RP_VecL) * 0.5;
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
    
    UE_LOG(LogMyGame, Display, TEXT("%s: OnComponentCreated"), *GetFullName())
    GyrationSetup();
}

void UGyrationComponent::PostLoad()
{
    Super::PostLoad();
    UE_LOG(LogMyGame, Display, TEXT("%s: PostLoad"), *GetFullName())

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
