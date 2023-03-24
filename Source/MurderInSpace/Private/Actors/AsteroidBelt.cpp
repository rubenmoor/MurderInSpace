﻿#include "Actors/AsteroidBelt.h"

#include "Actors/DynamicAsteroid.h"
#include "Components/SphereComponent.h"
#include "Modes/MyGameInstance.h"

AAsteroidBelt::AAsteroidBelt()
{
    PrimaryActorTick.bCanEverTick = false;
    bRunConstructionScriptOnDrag = false;
    bNetLoadOnClient = false;

    Root = CreateDefaultSubobject<USceneComponent>("Root");
    Root->SetMobility(EComponentMobility::Static);
	SetRootComponent(Root);
    
    Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
    Sphere->SetupAttachment(Root);
    Sphere->SetMobility(EComponentMobility::Static);
}

void AAsteroidBelt::Destroyed()
{
    Super::Destroyed();
    while(!Children.IsEmpty())
    {
        if(!Children[0]->Destroy())
        {
            UE_LOG(LogMyGame, Warning, TEXT("%s: Destroyed: Failed to destroy child actor %s.")
                , *GetFullName()
                , *Children[0].GetName()
                )
        }
    }
}

#if WITH_EDITOR
void AAsteroidBelt::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
    Super::PostEditChangeChainProperty(PropertyChangedEvent);

    const FName Name = PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName();
    
    static const FName FNameCurveAsteroidSize     = GET_MEMBER_NAME_CHECKED(AAsteroidBelt, CurveAsteroidSize    );
    static const FName FNameCurveAsteroidDistance = GET_MEMBER_NAME_CHECKED(AAsteroidBelt, CurveAsteroidDistance);
    static const FName FNameNumAsteroids          = GET_MEMBER_NAME_CHECKED(AAsteroidBelt, NumAsteroids         );
    static const FName FNameWidth                 = GET_MEMBER_NAME_CHECKED(AAsteroidBelt, Width                );

    if(     Name == FNameCurveAsteroidSize
         || Name == FNameCurveAsteroidDistance
         || Name == FNameNumAsteroids
         || Name == FNameWidth
        )
    {
        if(const auto* World = GetWorld(); IsValid(World) && World->WorldType != EWorldType::EditorPreview)
        {
            BuildAsteroids();
        }
    }
}

void AAsteroidBelt::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    auto* MyState = UMyState::Get();
    auto Physics = MyState->GetPhysicsAny(this);

    Sphere->SetWorldLocation(FVector::Zero());
    Sphere->SetSphereRadius((Transform.GetLocation() - Physics.VecF1).Length());
}
#endif


void AAsteroidBelt::BuildAsteroids()
{
    check(IsValid(CurveAsteroidSize))
    check(IsValid(CurveAsteroidDistance))
    check(!AsteroidTypes.IsEmpty())
    for(const auto AsteroidType : AsteroidTypes)
    {
        check(IsValid(AsteroidType.DynamicAsteroidClass))
    }
    
    while(!Children.IsEmpty())
    {
        if(IsValid(Children[0]))
        {
            Children[0]->Destroy();
        }
    }
    const FRandomStream RandomStream(GetFName());

    auto* World = GetWorld();
    check(World->WorldType != EWorldType::EditorPreview)
    
    auto* MyState = UMyState::Get();
    const FPhysics Physics = MyState->GetPhysicsAny(this);

    const FVector VecR = GetActorLocation();
    for(int i = 0; i < NumAsteroids; i++)
    {
        auto AsteroidType = PickAsteroidType(RandomStream);
        const FVector VecVaried = MakeAsteroidDistance(Physics, RandomStream);
        const float Radius = VecVaried.Length();
        const float AlphaZero = acos(VecVaried.X / Radius);
        //const float Alpha = 2 * PI * static_cast<float>(i) / NumAsteroids;
        const float Alpha = 2 * PI * RandomStream.FRand();
        const FVector VecLocation
            ( cos(AlphaZero + Alpha) * Radius
            , sin(AlphaZero + Alpha) * Radius
            , cos(Alpha) * VecR.Z
            );
        //const float SizeParam = MinAsteroidSize * MakeAsteroidSize(static_cast<float>(i) / NumAsteroids);
        FActorSpawnParameters SpawnParameters;
        // Owner is "primarily used for replication"; I use it to have the spawned asteroid
        // added to the children array for destruction
        SpawnParameters.Owner = this;
        SpawnParameters.CustomPreSpawnInitalization = [this, RandomStream, AsteroidType] (AActor* Actor)
        {
            // TODO: replace by some random distribution
            auto* DynamicAsteroid = Cast<ADynamicAsteroid>(Actor);
            DynamicAsteroid->SizeParam = MakeAsteroidSize(RandomStream, AsteroidType);
            DynamicAsteroid->RandomStream = RandomStream;
            //DynamicAsteroid->GenerateMesh(RandomStream, SizeParam);
            DynamicAsteroid->SetInitialOrbitParams
                ( { FVector(0.0, 0.0, 0.)
                    // TODO: only correct for VecR.Z == 0 
                , FVector(0., 0., 1.)
                });
#if WITH_EDITOR
            Actor->SetFolderPath(*GetName());
#endif
            // if(World->WorldType == EWorldType::Editor)
            // {
            //     // this isn't necessary when in game: AOrbit::BeginPlay calls this function, too
            //     auto* Orbit = ActorWithOrbit->GetOrbit();
            //     if(IsValid(Orbit)) Orbit->UpdateByInitialParams(Physics, InstanceUI);
            // }
        };
        World->SpawnActor<ADynamicAsteroid>
            ( AsteroidType.DynamicAsteroidClass
            , VecLocation
            , FRotator::ZeroRotator
            , SpawnParameters
            );
    }
}

float AAsteroidBelt::MakeAsteroidSize(const FRandomStream& RandomStream, const FAsteroidType AsteroidType) const
{
    return AsteroidType.MinAsteroidSize
        + AsteroidType.MaxAsteroidSize * CurveAsteroidSize->GetFloatValue(RandomStream.FRand());
}

FVector AAsteroidBelt::MakeAsteroidDistance(FPhysics Physics, const FRandomStream& RandomStream) const
{
    return (GetActorLocation() - Physics.VecF1) + Width * (CurveAsteroidDistance->GetFloatValue(RandomStream.FRand()) - 0.5);
}


FAsteroidType AAsteroidBelt::PickAsteroidType(FRandomStream RandomStream)
{
    float FreqSum = 0.;
    for(const auto AsteroidType : AsteroidTypes)
    {
        FreqSum += AsteroidType.RelativeFrequency;
    }
    const float Die = RandomStream.FRand() * FreqSum;

    FreqSum = 0.;
    for(const auto AsteroidType : AsteroidTypes)
    {
        FreqSum += AsteroidType.RelativeFrequency;
        if(Die < FreqSum)
        {
            return AsteroidType;
        }
    }
    //std::unreachable;
    return AsteroidTypes[0];
}

