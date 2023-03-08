#include "Actors/AsteroidBelt.h"
#include "Actors/DynamicAsteroid.h"
#include "Modes/MyGameInstance.h"

AAsteroidBelt::AAsteroidBelt()
{
    PrimaryActorTick.bCanEverTick = false;
    bRunConstructionScriptOnDrag = false;
    bNetLoadOnClient = false;
}

void AAsteroidBelt::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    // TODO: move all this code to PostEditChangeChainProperty and BeginPlay
    const auto* World = GetWorld();
    const auto WorldType=  World->WorldType;
    if  (  WorldType == EWorldType::EditorPreview
        || (WorldType == EWorldType::Editor && HasAnyFlags(RF_Transient))
        || ((WorldType == EWorldType::Game || WorldType == EWorldType::PIE) && !HasAnyFlags(RF_Transient))
        )
    {
        return;
    }
    
}

void AAsteroidBelt::Destroyed()
{
    Super::Destroyed();
    for(const auto Actor : Children)
    {
        Actor->Destroy();
    }
}

#if WITH_EDITOR
void AAsteroidBelt::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
    Super::PostEditChangeChainProperty(PropertyChangedEvent);

    const FName Name = PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName();
    
    static const FName FNameDynamicAsteroidClass = GET_MEMBER_NAME_CHECKED(AAsteroidBelt, DynamicAsteroidClass);
    static const FName FNameCurveAsteroidSize    = GET_MEMBER_NAME_CHECKED(AAsteroidBelt, CurveAsteroidSize   );
    static const FName FNameNumAsteroids         = GET_MEMBER_NAME_CHECKED(AAsteroidBelt, NumAsteroids        );
    static const FName FNameMinAsteroidSize      = GET_MEMBER_NAME_CHECKED(AAsteroidBelt, MinAsteroidSize     );
    static const FName FNameFractalNumber        = GET_MEMBER_NAME_CHECKED(AAsteroidBelt, FractalNumber       );

    switch(Name)
    {
    case FNameDynamicAsteroidClass:
    case FNameCurveAsteroidSize:
    case FNameNumAsteroids:
    case FNameMinAsteroidSize:
    case FNameFractalNumber:
        for(auto Actor : Children)
        {
            if(!Actor->Destroy())
            {
                UE_LOG(LogMyGame, Error, TEXT("%s: Failed to destroy child actor %s.")
                    , *GetFullName()
                    , *Actor.GetName()
                    )
            }
        }
        BuildAsteroids();
        break;
    default:
        break;
    }
}
#endif


void AAsteroidBelt::BuildAsteroids()
{
    check(IsValid(CurveAsteroidSize))
    check(IsValid(DynamicAsteroidClass))

    auto* World = GetWorld();
    auto* MyState = UMyState::Get();
    const FPhysics Physics = MyState->GetPhysicsAny(this);
    const FInstanceUI InstanceUI = MyState->GetInstanceUIAny(this);

    const FVector VecR = GetActorLocation();
    const float Radius = (VecR - Physics.VecF1).Length();
    const float AlphaZero = acos(VecR.X / Radius);
    for(int i = 0; i < NumAsteroids; i++)
    {
        const float Alpha = static_cast<float>(i) / NumAsteroids * 2. * PI;
        const float X = cos(AlphaZero + Alpha) * Radius;
        const float Y = sin(AlphaZero + Alpha) * Radius;
        const float Z = cos(Alpha) * VecR.Z;
        const FVector VecLocation(X, Y, Z);
        //const float SizeParam = MinAsteroidSize * MakeAsteroidSize(static_cast<float>(i) / NumAsteroids);
        const float SizeParam = MakeAsteroidSize(static_cast<float>(i));
        // abusing the scale.x of Transform to pass a size parameter to the asteroid
        const FVector VecScale(SizeParam, 1., 1.);
        const FTransform AsteroidTransform(FQuat::Identity, VecLocation, VecScale);
        FActorSpawnParameters SpawnParameters;
        // Owner is "primarily used for replication"; I use it to have the spawned asteroid
        // added to the children array for destruction
        SpawnParameters.Owner = this;
        SpawnParameters.CustomPreSpawnInitalization = [Physics, InstanceUI, World] (AActor* Actor)
        {
            // TODO: replace by some random distribution
            auto* ActorWithOrbit = Cast<IHasOrbit>(Actor);
            ActorWithOrbit->SetInitialOrbitParams
                ( { FVector(0.0, 0.0, 0.)
                    // TODO: only correct for VecR.Z == 0 
                , FVector(0., 0., 1.)
                });
            if(World->WorldType == EWorldType::Editor)
            {
                // this isn't necessary when in game: AOrbit::BeginPlay calls this function, too
                //auto* Orbit = ActorWithOrbit->GetOrbit();
                //if(IsValid(Orbit)) Orbit->UpdateByInitialParams(Physics, InstanceUI);
                ActorWithOrbit->GetOrbit()->UpdateByInitialParams(Physics, InstanceUI);
            }
        };
        GetWorld()->SpawnActor<ADynamicAsteroid>
            ( DynamicAsteroidClass
            , AsteroidTransform
            , SpawnParameters
            );
    }
}

float AAsteroidBelt::FractalNoise(int32 N, float Seed)
{
    float Result = Seed;
    float X = Result;
    for(int i = 0; i < N; i++)
    {
        Result = FMath::PerlinNoise1D(X);
        X = (Result + 1.) / 2.;
    }
    return Result;
}

float AAsteroidBelt::MakeAsteroidSize(float Seed) const
{
    return MinAsteroidSize * CurveAsteroidSize->GetFloatValue(FractalNoise(FractalNumber, Seed));
}
