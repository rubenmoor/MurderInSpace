#include "Actors/AsteroidBelt.h"

#include "NiagaraComponent.h"
#include "Actors/DynamicAsteroid.h"
#include "Components/SphereComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Modes/MyGameInstance.h"

AAsteroidBelt::AAsteroidBelt()
{
    PrimaryActorTick.bCanEverTick = false;
    bNetLoadOnClient = false;

#if WITH_EDITOR
    bRunConstructionScriptOnDrag = false;
#endif

    Root = CreateDefaultSubobject<USceneComponent>("Root");
    Root->SetMobility(EComponentMobility::Static);
	SetRootComponent(Root);

    Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
    Sphere->SetupAttachment(Root);
    Sphere->SetMobility(EComponentMobility::Static);

    NS_Fog = CreateDefaultSubobject<UNiagaraComponent>("NS_Fog");
    NS_Fog->SetupAttachment(Root);
    
    RandomStream.Initialize(GetFName());
}

void AAsteroidBelt::Destroyed()
{
    Super::Destroyed();
    while(!Asteroids.IsEmpty())
    {
        if(IsValid(Asteroids[0]))
        {
            if(!Asteroids[0]->Destroy())
            {
                UE_LOG(LogMyGame, Warning, TEXT("%s: Destroyed: Failed to destroy asteroid %s.")
                    , *GetFullName()
                    , *Asteroids[0]->GetName()
                    )
                Asteroids.RemoveAt(0);
            }
        }
        else
        {
            Asteroids.RemoveAt(0);
            UE_LOG(LogMyGame, Warning, TEXT("%s: invalid asteroid in Asteroids. Removing from array")
                , *GetFullName())
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
    
    static const FName FNameFogDensity            = GET_MEMBER_NAME_CHECKED(AAsteroidBelt, FogDensity           );
    static const FName FNameParticleSizeMax       = GET_MEMBER_NAME_CHECKED(AAsteroidBelt, ParticleSizeMax      );
    static const FName FNameParticleSizeMin       = GET_MEMBER_NAME_CHECKED(AAsteroidBelt, ParticleSizeMin      );

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

    if(
            Name == FNameFogDensity
         || Name == FNameParticleSizeMax
         || Name == FNameParticleSizeMin
         || Name == FNameWidth
         )
    {
        const double Radius = Sphere->GetUnscaledSphereRadius();
        
        NS_Fog->SetFloatParameter("ParticleSizeMax", ParticleSizeMax);
        NS_Fog->SetFloatParameter("ParticleSizeMin", ParticleSizeMin);
        NS_Fog->SetFloatParameter("HandleRadius", std::max(0., 0.25 * Width - ParticleSizeMax));
        NS_Fog->SetIntParameter("SpawnCount", 2 * UE_PI * Radius * FogDensity);
    }
}

void AAsteroidBelt::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    auto* MyState = UMyState::Get();
    const auto Physics = MyState->GetPhysicsAny(this);
    
    const FVector VecR = Transform.GetLocation();
    Sphere->SetWorldLocation(FVector::Zero());
    Sphere->SetSphereRadius((VecR - Physics.VecF1).Length());

    NS_Fog->SetWorldLocation(FVector::Zero());
    
    NS_Fog->SetFloatParameter("ParticleSizeMax", ParticleSizeMax);
    NS_Fog->SetFloatParameter("ParticleSizeMin", ParticleSizeMin);
    NS_Fog->SetFloatParameter("HandleRadius", std::max(0., 0.25 * Width - ParticleSizeMax));

    const double Radius = Sphere->GetUnscaledSphereRadius();
    NS_Fog->SetFloatParameter("LargeRadius", Radius);
    NS_Fog->SetIntParameter("SpawnCount", 2 * UE_PI * Radius * FogDensity);
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
    
    while(!Asteroids.IsEmpty())
    {
        if(IsValid(Asteroids[0]))
        {
            Asteroids[0]->Destroy();
        }
    }

    RandomStream.Reset();
    
    auto* World = GetWorld();
    check(World->WorldType != EWorldType::EditorPreview)
    
    auto* MyState = UMyState::Get();
    const FPhysics Physics = MyState->GetPhysicsAny(this);

    const FVector VecR = GetActorLocation();
    for(int i = 0; i < NumAsteroids; i++)
    {
        auto AsteroidType = PickAsteroidType();
        const FVector VecVaried = MakeAsteroidDistance(Physics);
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
        SpawnParameters.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
        SpawnParameters.CustomPreSpawnInitalization = [this, AsteroidType] (AActor* Actor)
        {
            // TODO: replace by some random distribution
            auto* DynamicAsteroid = Cast<ADynamicAsteroid>(Actor);
            DynamicAsteroid->Initialize
                ( MakeAsteroidSize (AsteroidType)
                , MeshResolution
                , bRecomputeNormals
                , RandomStream.GetUnsignedInt()
                , this
                );
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
        auto* Asteroid= World->SpawnActor<ADynamicAsteroid>
            ( AsteroidType.DynamicAsteroidClass
            , VecLocation
            , FRotationMatrix::MakeFromX(RandomStream.VRand()).Rotator()
            , SpawnParameters
            );
        Asteroids.Add(Asteroid);
    }
}

float AAsteroidBelt::MakeAsteroidSize(const FAsteroidType& AsteroidType) const
{
    return AsteroidType.MinAsteroidSize
        + AsteroidType.MaxAsteroidSize * CurveAsteroidSize->GetFloatValue(RandomStream.FRand());
}

FVector AAsteroidBelt::MakeAsteroidDistance(FPhysics Physics) const
{
    return (GetActorLocation() - Physics.VecF1) + Width * (CurveAsteroidDistance->GetFloatValue(RandomStream.FRand()) - 0.5);
}


FAsteroidType AAsteroidBelt::PickAsteroidType()
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

