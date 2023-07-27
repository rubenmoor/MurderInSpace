#include "Actors/AsteroidBelt.h"

#include "NiagaraComponent.h"
#include "Actors/DynamicAsteroid.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Logging/StructuredLog.h"
#include "Modes/MyGameInstance.h"
#include "Modes/MyGameState.h"

AAsteroidBelt::AAsteroidBelt(): AActor()
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
                UE_LOGFMT(LogMyGame, Warning, "{0}: Destroyed: Failed to destroy asteroid {1}."
                    , GetFName()
                    , Asteroids[0]->GetName()
                    );
                Asteroids.RemoveAt(0);
            }
        }
        else
        {
            Asteroids.RemoveAt(0);
            UE_LOGFMT(LogMyGame, Warning, "{0}: invalid asteroid in Asteroids. Removing from array"
                , GetFName());
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
    static const FName FNameEnabled               = GET_MEMBER_NAME_CHECKED(AAsteroidBelt, bEnabled             );

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
    else if
        ( Name == FNameEnabled
        )
    {
        for(auto* Asteroid : Asteroids)
        {
            Asteroid->SetActorEnableCollision(bEnabled);
            Asteroid->GetRootComponent()->SetVisibility(bEnabled, true);
            Asteroid->GetOrbit()->SetEnabled(bEnabled);
        }
    }
}

void AAsteroidBelt::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    auto* GS = AMyGameState::Get(this);
    const auto Physics = IsValid(GS) ? GS->RP_Physics : FPhysics();
    
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
    check(!AsteroidGroups.IsEmpty())
    for(const auto AsteroidGroup : AsteroidGroups)
    {
        check(IsValid(AsteroidGroup.DynamicAsteroidClass))
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

    auto* GS = AMyGameState::Get(this);
    const FPhysics Physics = IsValid(GS) ? GS->RP_Physics : FPhysics();

    const FVector VecR = GetActorLocation();
    for(int i = 0; i < NumAsteroids; i++)
    {
        auto AsteroidGroup = PickAsteroidGroup();
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
        SpawnParameters.CustomPreSpawnInitalization = [this, AsteroidGroup] (AActor* Actor)
        {
            // TODO: replace by some random distribution
            auto* DynamicAsteroid = Cast<ADynamicAsteroid>(Actor);
            DynamicAsteroid->Initialize
                ( MakeAsteroidSize (AsteroidGroup)
                , MeshResolution
                , bRecomputeNormals
                , RandomStream.GetUnsignedInt()
                , this
                );
            DynamicAsteroid->SetInitialOrbitParams
                ( { FVector(0.0, 0.0, 0.)
                // TODO: only correct for VecR.Z == 0 
                , FVector(0., 0., 1.)
                // for line/line-bound orbit: set velocity
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
            ( AsteroidGroup.DynamicAsteroidClass
            , VecLocation
            , UKismetMathLibrary::RandomRotator(true)
            , SpawnParameters
            );
        Asteroids.Add(Asteroid);
    }
}

float AAsteroidBelt::MakeAsteroidSize(const FAsteroidGroup& AsteroidGroup) const
{
    return AsteroidGroup.MinAsteroidSize
        + AsteroidGroup.MaxAsteroidSize * CurveAsteroidSize->GetFloatValue(RandomStream.FRand());
}

FVector AAsteroidBelt::MakeAsteroidDistance(FPhysics Physics) const
{
    return (GetActorLocation() - Physics.VecF1) + Width * (CurveAsteroidDistance->GetFloatValue(RandomStream.FRand()) - 0.5);
}


FAsteroidGroup AAsteroidBelt::PickAsteroidGroup()
{
    float FreqSum = 0.;
    for(const auto AsteroidGroup : AsteroidGroups)
    {
        FreqSum += AsteroidGroup.RelativeFrequency;
    }
    const float Die = RandomStream.FRand() * FreqSum;

    FreqSum = 0.;
    for(const auto AsteroidGroup : AsteroidGroups)
    {
        FreqSum += AsteroidGroup.RelativeFrequency;
        if(Die < FreqSum)
        {
            return AsteroidGroup;
        }
    }
    //std::unreachable;
    return AsteroidGroups[0];
}

