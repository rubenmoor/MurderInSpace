#include "Actors/AsteroidBelt.h"
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
    
    static const FName FNameDynamicAsteroidClass = GET_MEMBER_NAME_CHECKED(AAsteroidBelt, DynamicAsteroidClass);
    static const FName FNameCurveAsteroidSize    = GET_MEMBER_NAME_CHECKED(AAsteroidBelt, CurveAsteroidSize   );
    static const FName FNameNumAsteroids         = GET_MEMBER_NAME_CHECKED(AAsteroidBelt, NumAsteroids        );
    static const FName FNameMinAsteroidSize      = GET_MEMBER_NAME_CHECKED(AAsteroidBelt, MinAsteroidSize     );
    static const FName FNameFractalNumber        = GET_MEMBER_NAME_CHECKED(AAsteroidBelt, FractalNumber       );

    if(     Name == FNameDynamicAsteroidClass
         || Name == FNameCurveAsteroidSize
         || Name == FNameNumAsteroids
         || Name == FNameMinAsteroidSize
         || Name == FNameFractalNumber
        )
    {
        if(const auto* World = GetWorld(); IsValid(World) && World->WorldType != EWorldType::EditorPreview)
        {
            while(!Children.IsEmpty())
            {
                Children[0]->Destroy();
            }
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
    check(IsValid(DynamicAsteroidClass))

    FRandomStream RandomStream(GetFName());

    auto* World = GetWorld();
    check(World->WorldType != EWorldType::EditorPreview)
    
    auto* MyState = UMyState::Get();
    const FPhysics Physics = MyState->GetPhysicsAny(this);

    const FVector VecR = GetActorLocation();
    const float Radius = (VecR - Physics.VecF1).Length();
    const float AlphaZero = acos(VecR.X / Radius);
    for(int i = 0; i < NumAsteroids; i++)
    {
        const float Alpha = static_cast<float>(i) / NumAsteroids * 2. * PI;
        const FVector VecLocation
            ( cos(AlphaZero + Alpha) * Radius
            , sin(AlphaZero + Alpha) * Radius
            , cos(Alpha) * VecR.Z
            );
        //const float SizeParam = MinAsteroidSize * MakeAsteroidSize(static_cast<float>(i) / NumAsteroids);
        const float SizeParam = MakeAsteroidSize(RandomStream);
        FActorSpawnParameters SpawnParameters;
        // Owner is "primarily used for replication"; I use it to have the spawned asteroid
        // added to the children array for destruction
        SpawnParameters.Owner = this;
        SpawnParameters.CustomPreSpawnInitalization = [this, SizeParam] (AActor* Actor)
        {
            // TODO: replace by some random distribution
            auto* DynamicAsteroid = Cast<ADynamicAsteroid>(Actor);
            DynamicAsteroid->GenerateMesh(SizeParam);
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
            ( DynamicAsteroidClass
            , VecLocation
            , FRotator::ZeroRotator
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

float AAsteroidBelt::MakeAsteroidSize(const FRandomStream& RandomStream) const
{
    return MinAsteroidSize + MaxAsteroidSize * CurveAsteroidSize->GetFloatValue(RandomStream.FRand());
}
